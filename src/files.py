from typing import Tuple

import numpy as np
import librosa

from constants import *


def load_file(name: str) -> Tuple[np.ndarray, int]:
    return librosa.load(name, sr=SAMPLING_RATE)


def audio_length(audio: np.ndarray) -> int:
    return audio.shape[0] / SAMPLING_RATE


