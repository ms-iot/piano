using System;
using System.Threading.Tasks;
using Windows.Devices.Spi;

/*
KeyboardLightController Static Library

This library will control keyboard lighting
using pins 1-14 (ignoring 0 and 15) of the
MCP23S17 GPIO port expander via SPI.

SPI requires the following pins:
SS - Slave select
MOSI - Master out Slave in
SCK - Serial Clock

The circuit:
MCP23S17 - GPIO (general purpose input/output) port expander
14 N-channel MOSFETs - One for each LED, triggering 12V/1A of current.
LED Strip - RadioShack LED Waterproof Flexi Strip 60 LED 1M (White)
12V/5A power supply
*/

namespace build2014_piano
{
    class KeyboardLightController
    {
        public enum SetAction
        {
            Disable,
            Enable,
            Force
        }

        //%%%%%%%%%%%/ MCP23S17 Op-codes /%%%%%%%%%%%//
        const byte SPI_SLAVE_ID = 0x40;    //0b01000000
        const byte SPI_SLAVE_ADDR = 0x00;  //0b00000000
        const byte SPI_SLAVE_WRITE = 0x00; //0b00000000

        //%%/ MCP23S17 Control Register Addresses /%%//
        const byte CTLREG_IODIRA = 0x00;   //0b00000000
        const byte CTLREG_IODIRB = 0x01;   //0b00000001
        const byte CTLREG_GPIOA = 0x12;    //0b00010010
        const byte CTLREG_GPIOB = 0x13;    //0b00010011

        public KeyboardLightController(SpiDevice spi_) {
            _spi = spi_;
        }

        /// <summary>
        /// Sets up the MCP23S17 for the Keyboard Lighting
        /// </summary>
        public void Begin() {
            // Set GPIO pins to OUTPUT
            _spi?.Write(new byte[] { (SPI_SLAVE_ID | SPI_SLAVE_ADDR | SPI_SLAVE_WRITE), CTLREG_IODIRA, 0x00, 0x00 });
        }

        /// <summary>
        /// This will turn off the lights indicated by the bitmask
        /// </summary>
        /// <param name="bitmask_"></param>
        /// <returns></returns>
        public ushort DisableLights(ushort bitmask_) {
            return Set(bitmask_, SetAction.Disable);
        }

        /// <summary>
        /// This will turn on the lights indicated by the bitmask
        /// </summary>
        /// <param name="bitmask_"></param>
        /// <returns></returns>
        public ushort EnableLights(ushort bitmask_) {
            return Set(bitmask_, SetAction.Enable);
        }

        /// <summary>
        /// To be called when ending the SPI communication with the keyboard lights
        /// </summary>
        public void End () {
            // Set GPIO pins to INPUT
            _spi?.Write(new byte[] { (SPI_SLAVE_ID | SPI_SLAVE_ADDR | SPI_SLAVE_WRITE), CTLREG_IODIRA, 0xFF, 0xFF });
        }

        /// <summary>
        /// Alternates flashing of the status lights on 0 and 15 to provide a heartbeat
        /// </summary>
        public void Heartbeat () {
            byte hi = 0x00;
            byte lo = 0x00;

            // Set heartbeat
            if (_blink_bank_a)
            {
                lo = 0x01;
            }
            else
            {
                hi = 0x80;
            }
            _blink_bank_a = !_blink_bank_a;

            TransmitStateUsingHiLoHeartbeatMask(hi, lo);
        }

        /// <summary>
        /// Sets the lights on and off as designated by the bitmask
        /// </summary>
        /// <param name="bitmask_"></param>
        /// <returns></returns>
        public ushort SetLights(ushort bitmask_) {
            return Set(bitmask_, SetAction.Force);
        }

        private bool _blink_bank_a = false;
        private object _mutex = new object();
        private SpiDevice _spi = null;
        private ushort _state = 0x0000;

        /// <summary>
        /// Sets the lights given by the bitmask based on the action parameter (SetAction)
        /// </summary>
        /// <param name="bitmask_"></param>
        /// <param name="action_"></param>
        /// <returns></returns>
        /// <remarks>
        /// Note: Does not affect pins 0 and 15
        /// </remarks>
        private ushort Set (ushort bitmask_, SetAction action_) {
            // Store heartbeat mask
            byte hi = (byte)((_state >> 8) & 0x80);
            byte lo = (byte)(_state & 0x01);

            lock (_mutex)
            {
                // Apply bitmask to state
                switch (action_)
                {
                    case SetAction.Disable:
                        _state &= (ushort)(~bitmask_);
                        break;
                    case SetAction.Enable:
                        _state |= bitmask_;
                        break;
                    case SetAction.Force:
                        _state = bitmask_;
                        break;
                    default:
                        break;
                }
            }

            return TransmitStateUsingHiLoHeartbeatMask(hi, lo);
        }


        /// <summary>
        /// Rolls a light bar (five wide) across the pins
        /// </summary>
        async public
        void Shimmer()
        {
            ushort key_mask = 0x0000;
            for (int i = -4; i < 15; ++i)
            {
                ushort light_bar = 0x003E;
                ushort user_mask = (ushort)(~key_mask & _state);
                int shift_count = Math.Abs(i);
                if (i < 0)
                {
                    key_mask = (ushort)(light_bar >> shift_count);
                }
                else
                {
                    key_mask = (ushort)(light_bar << shift_count);
                }
                SetLights((ushort)(key_mask | user_mask));

                await Task.Delay(100);
            }
        }

        /// <summary>
        /// Applies heartbeat to register values and transmits on the spi bus
        /// </summary>
        /// <param name="hi_"></param>
        /// <param name="lo_"></param>
        /// <returns></returns>
        private ushort TransmitStateUsingHiLoHeartbeatMask (byte hi_, byte lo_)
        {
            // Ensure valid heartbeat mask
            byte hi = (byte)(hi_ & 0x80);
            byte lo = (byte)(lo_ & 0x01);

            // Break into two bytes for transmission and apply heartbeat mask
            hi |= (byte)(_state >> 8);
            lo |= (byte)(_state);

            // Reset heartbeat
            _state >>= 1;
            _state <<= 2;
            _state >>= 1;

            // Transmit bytes
            _spi?.Write(new byte[] { (SPI_SLAVE_ID | SPI_SLAVE_ADDR | SPI_SLAVE_WRITE), CTLREG_GPIOA, lo, hi });

            return _state;
        }
    }
}
