using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading.Tasks;
using System.Xml.Linq;
using Windows.ApplicationModel;
using Windows.Devices.Enumeration;
using Windows.Devices.Midi;
using Windows.Devices.Spi;
using Windows.Storage;
using Windows.System;
using Windows.System.Threading;
using Windows.UI.Core;
using Windows.UI.ViewManagement;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Input;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

namespace build2014_piano
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainPage : Page
    {
        bool[] _activeKeys;
        Dictionary<char, int> _charToNoteIndex;
        ThreadPoolTimer _demo;
        KeyboardLightController _lights;
        MidiPiano _piano;
        bool _spiAvailable;
        SpiDevice _spi;

        public
        MainPage()
        {
            InitializeComponent();
            Initialize();
        }

        private
        void CoreWindow_KeyDown(object sender_, KeyEventArgs e_)
        {
            KeyboardInput.Text = "" + Convert.ToChar(e_.VirtualKey);
            int index = VirtualKeyToIndex(e_.VirtualKey);
            if ( index < 0 ) { return; }
            ProgrammaticKeyPressAtIndex(index);
        }

        private
        void CoreWindow_KeyUp(object sender_, KeyEventArgs e_)
        {
            KeyboardInput.Text = "";
            int index = VirtualKeyToIndex(e_.VirtualKey);
            if (index < 0) { return; }
            ProgrammaticKeyReleaseAtIndex(index);
        }

        private
        void DisableLightAtIndex (int index_)
        {
            if (_spiAvailable)
            {
                _demo.Cancel();
                _lights?.DisableLights(Convert.ToUInt16(1 << (index_ + 1)));
                _demo = ThreadPoolTimer.CreatePeriodicTimer(Shimmer, TimeSpan.FromSeconds(10));
            }
        }

        private
        void EnableLightAtIndex(int index_)
        {
            if (_spiAvailable)
            {
                _demo.Cancel();
                _lights?.EnableLights(Convert.ToUInt16(1 << (index_ + 1)));
                _demo = ThreadPoolTimer.CreatePeriodicTimer(Shimmer, TimeSpan.FromSeconds(15));
            }
        }

        async private 
        void Initialize ()
        {
            await InitializeDependencies();
            InitializeContent();
            if (_spiAvailable) {
                Shimmer(null);
                ThreadPoolTimer.CreatePeriodicTimer(InvokeHeartbeat, TimeSpan.FromSeconds(1));
                _demo = ThreadPoolTimer.CreatePeriodicTimer(Shimmer, TimeSpan.FromSeconds(15));
            }
        }

        private
        void InitializeContent()
        {
            ApplicationView.PreferredLaunchWindowingMode = ApplicationViewWindowingMode.FullScreen;
#if DEBUG
            Canvas.SetZIndex(InputBorder, 3);
#endif
            Window.Current.CoreWindow.KeyDown += CoreWindow_KeyDown;
            Window.Current.CoreWindow.KeyUp += CoreWindow_KeyUp;

            for (int i = 0; i < _piano.NoteCount; ++i)
            {
                string buttonName = "WhiteKey" + i;
                Button b = FindName(buttonName) as Button;
                b.AddHandler(PointerPressedEvent, new PointerEventHandler(WhiteKey_PointerPressed), true);
                b.AddHandler(PointerReleasedEvent, new PointerEventHandler(WhiteKey_PointerReleased), true);
            }
        }

        async private
        Task InitializeDependencies()
        {
            await LoadMidiDataFromFileAsync();
            _activeKeys = new bool[_piano.NoteCount];

            // Initialize Piano resources
            await _piano.BeginAsync();

            // Initialize Keyboard lights (if SPI is available)
            try
            {
                string advancedQueryString = SpiDevice.GetDeviceSelector("SPI0");
                DeviceInformationCollection deviceInformationCollection = await DeviceInformation.FindAllAsync(advancedQueryString);
                _spiAvailable = (deviceInformationCollection.Count > 0);
                string deviceId = deviceInformationCollection[0].Id;

                SpiConnectionSettings spiConnection = new SpiConnectionSettings(0);
                spiConnection.ClockFrequency = 4000000;
                spiConnection.DataBitLength = 8;
                spiConnection.Mode = SpiMode.Mode0;
                spiConnection.SharingMode = SpiSharingMode.Shared;

                _spi = await SpiDevice.FromIdAsync(deviceId, spiConnection);
                _lights = new KeyboardLightController(_spi);
                _lights?.Begin();
                _spiAvailable = true;
            }
            catch (Exception)
            {
                _spiAvailable = false;
            }
        }

        private
        void InvokeHeartbeat(ThreadPoolTimer timer_)
        {
            _lights?.Heartbeat();
        }

        async private
        Task LoadMidiDataFromFileAsync()
        {
            // Load key mapping XML file
            StorageFile keyMapXml = await LoadKeyMapAsync();
            Debug.Assert(null != keyMapXml);

            // Parse XML file
            string fileText = await FileIO.ReadTextAsync(keyMapXml);
            XDocument xml = XDocument.Parse(fileText);
            IEnumerable<XElement> noteElements = xml.Descendants("Note");

            // Load state variables
            int key_index = 0;
            _charToNoteIndex = new Dictionary<char, int>();
            List<MidiNoteOnMessage> notes = new List<MidiNoteOnMessage>();
            foreach (XElement note in noteElements)
            {
                if ( !note.HasAttributes ) { continue; }
                _charToNoteIndex.Add(note.Attribute("Key").Value.ToCharArray()[0], key_index++);
                byte channel = Convert.ToByte(note.Attribute("Channel").Value);
                byte midiNote = Convert.ToByte(note.Attribute("Note").Value);
                byte velocity = Convert.ToByte(note.Attribute("Velocity").Value);
                notes.Add(new MidiNoteOnMessage(channel, midiNote, velocity));
            }

            _piano = new MidiPiano(notes.ToArray());
        }

        async private
        Task<StorageFile> LoadKeyMapAsync ()
        {
            // Load from USB thumb drive if available
            try
            {
                return await (await KnownFolders.RemovableDevices.GetFolderAsync("E:\\")).GetFileAsync("KeyMap.xml");
            }
            catch (Exception removable_media_exception)
            {
                Debug.WriteLine(removable_media_exception.Message);
                // Load default notes from package
                try
                {
                    return (await Package.Current.InstalledLocation.GetFileAsync(@"Assets\KeyMap.xml"));
                }
                catch (Exception package_exception)
                {
                    Debug.WriteLine(package_exception.Message);
                    return null;
                }
            }
        }

        private
        void ProgrammaticKeyPressAtIndex(int index_)
        {
            lock (_activeKeys)
            {
                if (_activeKeys[index_]) { return; }
                _activeKeys[index_] = true;
            }

            Button b = FindName("WhiteKey" + index_) as Button;
            VisualStateManager.GoToState(b, "Pressed", true);
            EnableLightAtIndex(index_);
            _piano.PlayNoteAtIndex(index_);
        }

        private
        void ProgrammaticKeyReleaseAtIndex(int index_)
        {
            lock (_activeKeys)
            {
                _activeKeys[index_] = false;
            }

            Button b = FindName("WhiteKey" + index_) as Button;
            VisualStateManager.GoToState(b, "Normal", true);
            DisableLightAtIndex(index_);
        }

        private
        void Shimmer (ThreadPoolTimer timer_)
        {
            _lights?.Shimmer();
        }

        private
        int VirtualKeyToIndex(VirtualKey vKey_)
        {
            int index;
            char key = char.ToLower(Convert.ToChar(vKey_));
            if (!_charToNoteIndex.TryGetValue(key, out index)) { return -1; }
            return index;
        }

        private
        void WhiteKey_PointerPressed(object sender_, PointerRoutedEventArgs e_)
        {
            int index = -1;
            Button key = sender_ as Button;
            string name = key.Name;

            index = Convert.ToInt32(name.Substring("WhiteKey".Length));
            _piano.PlayNoteAtIndex(index);
            EnableLightAtIndex(index);
        }

        private
        void WhiteKey_PointerReleased(object sender_, PointerRoutedEventArgs e_)
        {
            Button key = sender_ as Button;
            string name = key.Name;

            DisableLightAtIndex(Convert.ToInt32(name.Substring("WhiteKey".Length)));
        }
    }
}
