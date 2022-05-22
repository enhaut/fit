from files import load_file, audio_length
from output import plot_audio
from output import generate_files

file, _ = load_file("audio/xdobro23.wav")
#print(audio_length(file))

#plot_audio(file)


generate_files(file)
