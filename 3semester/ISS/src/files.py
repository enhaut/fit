from typing import Tuple

import numpy as np
import librosa
from scipy.io.wavfile import write

from constants import *


def load_file(name: str) -> Tuple[np.ndarray, int]:
    audio, sr = librosa.load(name, sr=SAMPLING_RATE)
    if sr != SAMPLING_RATE:
        raise NotImplementedError("Invalid sampling rate of input file")

    return audio, sr


def save_file(name: str, audio: np.ndarray):
    scaled = np.int16(audio / np.max(np.abs(audio)) * 32767)
    write(f"audio/{name}.wav", SAMPLING_RATE, scaled)


def audio_length(audio: np.ndarray) -> int:
    return audio.shape[0] / SAMPLING_RATE


