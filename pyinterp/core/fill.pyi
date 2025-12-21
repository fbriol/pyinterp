from typing import TypeVar

from ..type_hints import NDArray2DFloat64, NDArray2DFloat32
from . import config

_FloatArrayT = TypeVar("_FloatArrayT", NDArray2DFloat32, NDArray2DFloat64)

def loess(data: _FloatArrayT, config: config.fill.Loess) -> _FloatArrayT: ...
