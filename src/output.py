import matplotlib.pyplot as plt
from scipy.signal import iirfilter, lfilter, tf2zpk, freqz, find_peaks
from scipy.fft import fft, fftfreq
import numpy as np
from typing import Tuple, List

from constants import *
from files import audio_length, save_file


def round_freq(freq: int) -> str:
    return '{:,}'.format(freq).replace(',', ' ')


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
    plt.title("Audio signal")
    plt.xlabel("Time [s]")
    plt.savefig(f"report/{filename}.png")
    plt.clf()
    plt.cla()
    plt.close()


def normalize(audio: np.ndarray) -> np.ndarray:
    audio = audio - np.mean(audio)
    return audio / (max(audio.min(), audio.max(), key=abs))


def task_4_1(audio: np.ndarray):
    plot_audio(audio, "original")
    return f"""# Task 4.1 - load audio
To load input signal I have created function `load_file()` located in `files.py` which returns
array of 2 elements. The first one is audio signal stored in 1D array _(type `numpy.ndarray`)_,
the second element is sampling rate of audio file detected by function `load()` from `librosa` library, this sampling rate
is not used anymore because sampling rate of input audio is already known - `16 kHz`. If sampling rate of loaded
file is not `16 kHz` program raises `NotImplementedError`.  
  
Audio length is determined by following formula:

$$ length = samples / sampling\_rate $$

### Audio length: {audio_length(audio)} seconds  
### Audio samples: {round_freq(audio.shape[0])}  
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

    plt.title("Audio signal - frame: 40")
    plt.plot(time, frames[40])
    plt.xlabel("Samples $[n]$")

    plt.savefig(f"report/frame.png")
    plt.clf()
    plt.cla()
    plt.close()

    return """# Task 4.2 - normalization  

## Normalized signal:  
Normalization is implemented in function `output.normalize()`.  
![](report/normalized.png)  
Signal above is normalized to interval <-1, 1> using following pseudo-code:
```python
maximum_value = samples_array.maximum_value()
maximum_value = abs(maximum_value)

for sample in samples_array:
    sample = sample / maximum_value
```

## Periodic frame 
Signal is divided in to 130 chunks and each contains 1024 samples:
$$ chunks\_count = samples\_count // samples\_overlap  $$
That results that last 512 and first 512 samples of next chunk are the same.
To find periodic frame i chose pretty straight-forward method. I just plotted every frame
and have chosen one pretty periodic frame. Using following code:

![](images/spacer_100.png)

```python
time = np.arange(0, 1024, 1)

for i in range(0, columns - 1):
    plt.title(f"Audio signal - frame: {i}")
    plt.plot(time, normalized[i * SAMPLES_OVERLAP : (i * SAMPLES_OVERLAP) + 1024])
    plt.show()
```

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
    db.set_label('Spectral power density [dB]', rotation=270, labelpad=15)

    plt.gca().set_xlabel('Time [s]')
    plt.gca().set_ylabel('Frequency [Hz]')
    plt.gca().invert_yaxis()
    plt.gca().set_aspect(0.001)
    plt.tight_layout()

    plt.savefig(f"report/spectrogram.png")
    plt.clf()
    plt.cla()
    plt.close()

    return """# Task 4.4 {#task44}
Firstly, samples are "ordered" to 2D array `filtered_X`, it is just array of arrays, which contains 1024 samples each
with 512 samples overlapping. That means that last 512 and first 512 samples of following frame are the same.
For calculation is used faster version of DFT - FFT implemented in function [`fft()`](https://numpy.org/doc/stable/reference/generated/numpy.fft.fft.html)
in `numpy.fft` library.  
DFT coefficients are then modified by formula:
$$ P[k] = 10log_{10}|X[k]|^{2} $$

![](report/spectrogram.png)

"""


def get_peaks(audio: np.ndarray) -> List[float, ]:
    yf = np.abs(fft(audio))
    xf = np.abs(fftfreq(audio.size, 1 / SAMPLING_RATE))

    peaks, _ = find_peaks(yf, height=40)    # 40 because at graph of spectral analysis only peaks were higher than 40
    peaks = peaks[:int(peaks.size / 2)]     # peaks array are complex conjugated

    peak_freqs = []

    plt.plot(xf, yf)
    for peak in peaks:
        peak_freqs.append(xf[peak])
        plt.plot(xf[peak], yf[peak], "X")

    plt.savefig("report/peaks.png")
    plt.clf()
    plt.cla()
    plt.close()

    return peak_freqs


def task_4_5(audio):
    get_peaks(audio)
    return f"""# Task 4.5 

Firstly I tried to read peaks from spectrogram from [Task 4.4](#task44) but this method is not effective also not very accurate.
Because of that I decided to use function [`find_peaks()`](https://docs.scipy.org/doc/scipy/reference/generated/scipy.signal.find_peaks.html)
from `scipy.signal` library, function is used in `get_peaks()` located in `output.py` and it returns list of frequencies with peak.
At graph bellow we can see these frequencies, `X` marks peak that found `find_peaks()` function.

$$ f_1 = {round_freq(FOUND_F1)} Hz $$
$$ f_2 = {round_freq(FOUND_F2)} Hz $$
$$ f_3 = {round_freq(FOUND_F3)} Hz $$
$$ f_4 = {round_freq(FOUND_F4)} Hz $$

![](report/peaks.png)

"""


def generate_cos(freq: float, duration: float):
    time = np.arange(0, duration, 1/SAMPLING_RATE)
    frequencies = time * freq
    cos = np.cos((2 * np.pi) * frequencies)

    return time, cos


def spectrogram(values: np.ndarray, labels: Tuple[str, str, str], name: str):
    plt.specgram(values, Fs=SAMPLING_RATE)
    plt.gca().set_xlabel(labels[0])
    plt.gca().set_ylabel(labels[1])
    plt.show()

    db = plt.colorbar()
    db.set_label(labels[2], rotation=270, labelpad=15)

    plt.savefig(f"report/{name}.png")


def task_4_6() -> str:
    time, f1_cos = generate_cos(FOUND_F1, 4.16)
    _, f2_cos = generate_cos(FOUND_F2, 4.16)
    _, f3_cos = generate_cos(FOUND_F3, 4.16)
    _, f4_cos = generate_cos(FOUND_F4, 4.16)

    mixed = f1_cos + f2_cos + f3_cos + f4_cos

    spectrogram(mixed, ("Time [s]", "Frequency [Hz]", "Spectral power density [dB]"), "4cos")
    save_file("4cos", mixed)

    return f"""# Task 4.6
Generating signal of 4 mixed cosines is implemented as sum of cosine at frequency f1, cosine at frequency f2, ...  
For example:
```python
DURATION = 4.16
f1_cos = generate_cos({FOUND_F1}, DURATION)
f2_cos = generate_cos({FOUND_F2}, DURATION)
...
mixed_cosines = f1_cos + f2_cos + ...
``` 
For generating cosines I use function [`cos()`](https://numpy.org/doc/stable/reference/generated/numpy.cos.html)
from `numpy` library.
Spectrogram shows 4 mixed cosines functions at frequencies from [Task 4.5](#task-4.5).
![](report/4cos.png)
As we can see, 4 cosines are at the frequencies from [Task 4.5](#task-4.5).  
Generated signal is saved in `audio/4cos.wav` file.


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

        responses += f"| {round_freq(freq)} Hz ± 50 Hz | {rounded_b} | {rounded_a} |\n"

        h = lfilter(b, a, imp)

        x = i % 2
        y = int(i > 1)

        ax[x, y].stem(np.arange(IR_SIZE), h, basefmt=' ')
        ax[x, y].set_xlabel('$n$')
        ax[x, y].set_title(f'Impulse response of {round_freq(freq)} Hz band-stop $h[n]$')

        ax[x, y].grid(alpha=0.5, linestyle='dotted')

    plt.tight_layout()
    plt.savefig("report/impulse_responses.png")

    return f"""# Task 4.7.3 - band-stop filter
My filter consists of set 4 band-stop filters. Every filter filters single frequency from [Task 4.5](#task-4.5).
I am using function [`iirfilter`](https://docs.scipy.org/doc/scipy/reference/generated/scipy.signal.iirfilter.html)
with parameters `btype="bandstop"` and `output="ba"` from library `scipy.signal` to create each filter.  
Filtered band is always in interval:
$$ <F_i - 50, F_i + 50>; i Œ {1, 2, 3, 4} $$
For example for band-stop of {round_freq(FOUND_F1)} Hz frequency:
$$ <{round_freq(FOUND_F1 - 50)}, {round_freq(FOUND_F1 + 50)}> $$

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


def zero_poles_plot(ax, freq, z, p, _, legend_location="upper left"):
    # plotting zeros and poles taken from https://nbviewer.org/github/zmolikova/ISS_project_study_phase/blob/master/Zvuk_spektra_filtrace.ipynb
    ang = np.linspace(0, 2 * np.pi, 100)
    ax.plot(np.cos(ang), np.sin(ang))

    ax.scatter(np.real(z), np.imag(z), marker='o', facecolors='none', edgecolors='r', label='zeros')
    ax.scatter(np.real(p), np.imag(p), marker='x', color='g', label='poles')

    ax.set_xlabel('Real part $\mathbb{R}\{$z$\}$')
    ax.set_ylabel('Imaginary part $\mathbb{I}\{$z$\}$')
    ax.set_title(f"Bandstop for {round_freq(freq)} Hz ± 50 Hz")

    ax.grid(alpha=0.5, linestyle='dotted')
    ax.legend(loc=legend_location)


def task_4_8():
    _, ax = plt.subplots(2, 2, figsize=(8, 7))

    for i, freq in enumerate([FOUND_F1, FOUND_F2, FOUND_F3, FOUND_F4]):
        x = i % 2
        y = int(i > 1)

        b, a = bandstop(freq)
        zero_poles_plot(ax[x, y], freq, *tf2zpk(b, a))

    plt.tight_layout()
    plt.savefig("report/zeros_poles.png")

    # zoom of first
    _, plot = plt.subplots(1, 1, figsize=(4, 3))
    zero_poles_plot(plot, FOUND_F1, *tf2zpk(*bandstop(FOUND_F1)), "upper right")
    plot.set_xlim(xmin=0.9)
    plot.set_ylim(ymin=-0.6, ymax=0.6)
    plt.tight_layout()
    plt.savefig("report/zeros_poles_zoomed.png")

    return f"""# Task 4.8
Zeros and poles are calculated using function [`tf2zpk()`](https://docs.scipy.org/doc/scipy/reference/generated/scipy.signal.tf2zpk.html)
from `scipy.signal` library.

![](report/zeros_poles.png)

As we can see, zeros and poles are not much visible, here is zoomed plot of band-stop {round_freq(FOUND_F1)} Hz:  
![](report/zeros_poles_zoomed.png){"{ width=50% }"}


"""


def task_4_9():
    _, ax = plt.subplots(4, 2, figsize=(9, 10))

    for i, freq in enumerate([FOUND_F1, FOUND_F2, FOUND_F3, FOUND_F4]):
        b, a = bandstop(freq)

        w, h = freqz(b, a)
        ax[i, 0].plot(w / 2 / np.pi * SAMPLING_RATE, np.abs(h))
        ax[i, 0].set_xlabel('Frequency [Hz]')
        ax[i, 0].set_title('Magnitude of frequency response $|H(e^{j\omega})|$')
        ax[i, 0].set_ylabel(f"Band-stop filter for {round_freq(freq)} Hz")

        ax[i, 1].plot(w / 2 / np.pi * SAMPLING_RATE, np.angle(h))
        ax[i, 1].set_xlabel('Frequency [Hz]')
        ax[i, 1].set_title('Phase of frequency response$\mathrm{arg}\ H(e^{j\omega})$')

    plt.tight_layout()
    plt.savefig("report/chars.png")

    plt.figure(figsize=(4, 4))
    for i, freq in enumerate([FOUND_F1, FOUND_F2, FOUND_F3, FOUND_F4]):
        b, a = bandstop(freq)

        w, h = freqz(b, a)
        plt.plot(w / 2 / np.pi * SAMPLING_RATE, np.abs(h))

    plt.title("Frequency response of all the filters")
    plt.gca().set_xlabel("Frequency [Hz]")
    plt.savefig("report/chars_mixed.png")

    return """# Task 4.9
To calculate frequency response is used [`freqz()`](https://docs.scipy.org/doc/scipy/reference/generated/scipy.signal.freqz.html)
function from `scipy.signal` library.  
![](report/chars.png)  

![](report/chars_mixed.png)  

As we can see at the chart, filter drops to 0.0 at the frequencies
$$ f_i ; i Œ {1, 2, 3, 4} $$

"""


def task_4_10(audio):
    for i, freq in enumerate([FOUND_F1, FOUND_F2, FOUND_F3, FOUND_F4]):
        b, a = bandstop(freq)
        audio = lfilter(b, a, audio)

    save_file("clean_bandstop", audio)

    spectrogram(audio, ("Cas [s]", "...", "..."), "clean")

    return """# Task 4.10

![](report/clean.png)

At spectrogram bellow we can see, that frequencies f1, f2, f3 and f4 has been removed.
Frequencies are basically "silent" but before filters application they contain only `cos` noise.

"""


def generate_files(audio: np.ndarray):
    page_generators = [create_head(), task_4_1(audio), task_4_2(audio), task_4_4(audio), task_4_5(audio), task_4_6()]

    for i, generator in enumerate(page_generators):
        output = generator
        print(output)

        file = open(f"report/{i}.md", "w")
        file.write(output)
