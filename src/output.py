import matplotlib.pyplot as plt
import numpy as np

from constants import *
from files import audio_length


def create_head():
    return """![](images/logo.png)
![](images/spacer_100.png)

# BRNO UNIVERSITY OF TECHNOLOGY  
### FACULTY OF INFORMATION TECHNOLOGY  

---  

![](images/spacer_300.png)  

# ISS project 2021/2022  

![](images/spacer_300.png)  

## Author: Samuel Dobron (xdobro23)  
"""


def plot_audio(audio: np.ndarray, filename: str):
    time = np.arange(0, audio_length(audio), 1 / SAMPLING_RATE)
    plt.plot(time, audio)
    plt.savefig(f"report/{filename}.png")


def normalize(audio: np.ndarray) -> np.ndarray:
    audio = audio - np.mean(audio)
    return audio / (max(audio.min(), audio.max(), key=abs))


def task_4_1(audio: np.ndarray):
    plot_audio(audio, "original")
    return f"""# Task 4.1
### Audio length: {audio_length(audio)} seconds  
### Audio samples: {'{:,}'.format(audio.shape[0]).replace(',', ' ')}  
### Audio values
- Minimal: {audio.min():.4f}  
- Maximal: {audio.max():.4f}  

  
![](report/original.png)  
"""


def task_4_2(audio: np.ndarray):
    normalized = normalize(audio)
    plot_audio(normalized, "normalized")
    columns = normalized.size // SAMPLES_OVERLAP
    rows = SAMPLES_IN_COLUMN

    frames = list()
    for i in range(0, columns - 1):
        frames.append(normalized[i * SAMPLES_OVERLAP : (i*SAMPLES_OVERLAP)+1024])

    time = np.arange(0, 1024, 1)
    # TODO: detect periodic signal in frame automatically plt.plot(np.abs(np.fft.fft(signal1)))

    plt.title("uloha 4.2 - frame: 40")
    plt.plot(time, frames[40])

    plt.savefig(f"report/frame.png")
    plt.clf()
    plt.cla()
    plt.close()

    return """# Task 4.2  

# Normalized signal:  

![](report/normalized.png)  

# Periodic frame  
I have chosen frame 40 (actually 41 due to Python indexing)  
![](report/frame.png)  

"""


def generate_files(audio: np.ndarray):
    page_generators = [create_head(), task_4_1(audio), task_4_2(audio)]

    for i, generator in enumerate(page_generators):
        output = generator
        print(output)

        file = open(f"report/{i}.md", "w")
        file.write(output)
