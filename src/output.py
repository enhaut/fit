import matplotlib.pyplot as plt
from scipy.signal import iirfilter, lfilter
import numpy as np

from constants import *
from files import audio_length, save_file


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
    plt.clf()
    plt.cla()
    plt.close()


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


def task_4_4(audio: np.ndarray) -> str:
    normalized = normalize(audio)
    plot_audio(normalized, "normalized")
    columns = normalized.size // SAMPLES_OVERLAP

    frames = list()
    for i in range(0, columns - 1):
        frames.append(normalized[i * SAMPLES_OVERLAP: (i * SAMPLES_OVERLAP) + 1024])

    filtered_X = np.array([np.abs(np.fft.fft(x)) for x in frames])
    plt.figure(figsize=(5, 5))
    x = (10 * np.log(np.abs(filtered_X.T[:512] ** 2)))
    plt.imshow(x, extent=(0, audio_length(audio), SAMPLING_RATE // 2, 0))

    db = plt.colorbar()
    db.set_label('Spektralní hustota výkonu [dB]', rotation=270, labelpad=15)

    plt.gca().set_xlabel('Čas [s]')
    plt.gca().set_ylabel('Frekvence [Hz]')
    plt.gca().invert_yaxis()
    plt.gca().set_aspect(0.001)
    plt.tight_layout()

    plt.savefig(f"report/spectrogram.png")
    plt.clf()
    plt.cla()
    plt.close()

    return """# Task 4.4
![](report/spectrogram.png)

"""


def task_4_5(audio):
    """
    x[38] = -300  # 612,9 Hz
    x[76] = -300  # 1 225,8Hz
    x[113] = -300 # 1 822,6Hz
    x[151] = -300 # 2435,5Hz
    x[62] = 1000Hz
    """
    return """# Task 4.5
f_{1} = 612.9 Hz  
  
  
```python
x[38] = -300 
x[76] = -300
x[113] = -300
x[151] = -300
```

"""


def generate_cos(freq: float, duration: float):
    time = np.arange(0, duration, 1/SAMPLING_RATE)
    frequencies = time * freq
    cos = np.cos((2 * np.pi) * frequencies)

    return time, cos


def task_4_6() -> str:
    time, f1_cos = generate_cos(FOUND_F1, 4.16)
    _, f2_cos = generate_cos(FOUND_F2, 4.16)
    _, f3_cos = generate_cos(FOUND_F3, 4.16)
    _, f4_cos = generate_cos(FOUND_F4, 4.16)

    mixed = f1_cos + f2_cos + f3_cos + f4_cos

    plt.specgram(mixed, Fs=SAMPLING_RATE)
    plt.gca().set_xlabel('Čas [s]')
    plt.gca().set_ylabel('Frekvence [Hz]')
    plt.show()

    db = plt.colorbar()
    db.set_label('Spektralní hustota výkonu [dB]', rotation=270, labelpad=15)

    plt.savefig("report/4cos.png")
    save_file("4cos", mixed)

    return """# Task 4.6

![](report/4cos.png)

"""


def bandstop(freq, N=3):
    Fs = SAMPLING_RATE / 2  # iirfilter uses Nyquist's freqs
    lowcut = (freq - TRANSITION_WIDTH) / Fs
    highcut = (freq + TRANSITION_WIDTH) / Fs

    return iirfilter(N, [lowcut, highcut], RIPPLE, ATTENUATION, "bandstop", output="ba")


def task_4_7():
    responses = ""
    imp = [1, *np.zeros(IR_SIZE - 1)]  # jednotkovy impuls

    _, ax = plt.subplots(2, 2, figsize=(9, 6))

    for i, freq in enumerate([FOUND_F1, FOUND_F2, FOUND_F3, FOUND_F4]):
        b, a = bandstop(freq)

        rounded_b = [round(x, 5) if ROUND_COEFFICIENTS else x for x in b]
        rounded_a = [round(x, 5) if ROUND_COEFFICIENTS else x for x in a]

        responses += f"| {freq} Hz | {rounded_b} | {rounded_a} |\n"

        h = lfilter(b, a, imp)

        x = i % 2
        y = int(i > 1)

        ax[x, y].stem(np.arange(IR_SIZE), h, basefmt=' ')
        ax[x, y].set_xlabel('$n$')
        ax[x, y].set_title(f'Impulse response of {freq} Hz band-stop $h[n]$')

        ax[x, y].grid(alpha=0.5, linestyle='dotted')

    plt.tight_layout()
    plt.savefig("report/impulse_responses.png")

    return f"""# Task 4.7.3 - band-stop filter
...

## Coefficients
Coefficients bellow are rounded by Python's built in function `round()` to 5 decimal numbers.
To generate table without rounding just disable it in `constants.py` - `ROUND_COEFFICIENTS`.

| Band-stop frequency | b coefficients | a coefficients |
| ------------------- | -------------- | -------------- |
{responses}

## Impulse responses
_Coefficients used for calculating impulse responses are not rounded._
![](report/impulse_responses.png)

"""



def generate_files(audio: np.ndarray):
    page_generators = [create_head(), task_4_1(audio), task_4_2(audio), task_4_4(audio), task_4_5(audio), task_4_6()]

    for i, generator in enumerate(page_generators):
        output = generator
        print(output)

        file = open(f"report/{i}.md", "w")
        file.write(output)
