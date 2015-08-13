using System;
using System.Threading.Tasks;
using Windows.Devices.Midi;

namespace build2014_piano
{
    class MidiPiano
    {
        MidiSynthesizer midi;
        MidiNoteOnMessage[] _notes;

        public
        MidiPiano(MidiNoteOnMessage[] notes_)
        {
            _notes = notes_;
        }

        async public
        Task BeginAsync()
        {
            midi = await MidiSynthesizer.CreateAsync();
        }

        public
        int NoteCount
        {
            get { return _notes.Length; }
        }

        public
        void PlayNoteAtIndex (int index_)
        {
            if (index_ < 0 || index_ >= _notes.Length) { return; }

            midi.SendMessage(_notes[index_]);
        }
    }
}
