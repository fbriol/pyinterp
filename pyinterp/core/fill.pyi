from typing import TypeVar

from ..type_hints import NDArray2DFloat64, NDArray2DFloat32
from . import config

_FloatArrayT = TypeVar("_FloatArrayT", NDArray2DFloat32, NDArray2DFloat64)

def loess(data: _FloatArrayT, config: config.fill.Loess) -> _FloatArrayT: ...
def gauss_seidel(
    grid: _FloatArrayT, config: config.fill.GaussSeidel
) -> tuple[int, float]: ...
def multigrid(
    grid: _FloatArrayT, config: config.fill.Multigrid
) -> tuple[int, float]: ...
def fft_inpaint(
    grid: _FloatArrayT, config: config.fill.FFTInpaint
) -> tuple[int, float]: ...
