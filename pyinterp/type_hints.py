"""Type hints for pyinterp package."""

from types import GenericAlias
from typing import TYPE_CHECKING, Any, Literal, TypeAlias, TypeVar

import numpy as np


OneDim: TypeAlias = tuple[int]
TwoDims: TypeAlias = tuple[int, int]
ThreeDims: TypeAlias = tuple[int, int, int]
FourDims: TypeAlias = tuple[int, int, int, int]

if TYPE_CHECKING:
    Vector3Float32: TypeAlias = np.ndarray[
        tuple[Literal[3]],
        np.dtype[np.float32],
    ]
    Vector3Float64: TypeAlias = np.ndarray[
        tuple[Literal[3]],
        np.dtype[np.float64],
    ]
    NDArray1D: TypeAlias = np.ndarray[
        OneDim,
        np.dtype[Any],
    ]
    NDArray1DBool: TypeAlias = np.ndarray[
        OneDim,
        np.dtype[np.int8],
    ]
    NDArray1DDateTime64: TypeAlias = np.ndarray[
        OneDim,
        np.dtype[np.datetime64],
    ]
    NDArray1DFloat32: TypeAlias = np.ndarray[
        OneDim,
        np.dtype[np.float32],
    ]
    NDArray1DFloat64: TypeAlias = np.ndarray[
        OneDim,
        np.dtype[np.float64],
    ]
    NDArray1DInt32: TypeAlias = np.ndarray[
        OneDim,
        np.dtype[np.int32],
    ]
    NDArray1DInt64: TypeAlias = np.ndarray[
        OneDim,
        np.dtype[np.int64],
    ]
    NDArray1DStr: TypeAlias = np.ndarray[OneDim, np.dtype[np.str_]]
    NDArray1DTimeDelta64: TypeAlias = np.ndarray[
        OneDim,
        np.dtype[np.timedelta64],
    ]
    NDArray1DUInt32: TypeAlias = np.ndarray[
        OneDim,
        np.dtype[np.uint32],
    ]
    NDArray1DUInt8: TypeAlias = np.ndarray[
        OneDim,
        np.dtype[np.uint8],
    ]
    NDArray2DFloat32: TypeAlias = np.ndarray[
        TwoDims,
        np.dtype[np.float32],
    ]
    NDArray2DFloat64: TypeAlias = np.ndarray[
        TwoDims,
        np.dtype[np.float64],
    ]
    NDArray2DInt64: TypeAlias = np.ndarray[
        TwoDims,
        np.dtype[np.int64],
    ]
    NDArray2DStr: TypeAlias = np.ndarray[TwoDims, np.dtype[np.str_]]
    NDArray2DUInt64: TypeAlias = np.ndarray[
        TwoDims,
        np.dtype[np.int64],
    ]

    NDArray2DFloating: TypeAlias = np.ndarray[
        TwoDims,
        np.dtype[np.floating[Any]],
    ]
else:
    ScalarType_co = TypeVar("ScalarType_co", bound=np.generic, covariant=True)
    _DType = GenericAlias(
        np.dtype,
        (ScalarType_co,),
    )
    Vector3Float32 = GenericAlias(
        np.ndarray,
        (Literal[3], _DType[np.float32]),
    )
    Vector3Float64 = GenericAlias(
        np.ndarray,
        (Literal[3], _DType[np.float64]),
    )
    NDArray1D = GenericAlias(
        np.ndarray,
        (OneDim, _DType[Any]),
    )
    NDArray1DBool = GenericAlias(
        np.ndarray,
        (OneDim, _DType[np.int8]),
    )
    NDArray1DDateTime64 = GenericAlias(
        np.ndarray,
        (OneDim, _DType[np.datetime64]),
    )
    NDArray1DFloat32 = GenericAlias(
        np.ndarray,
        (OneDim, _DType[np.float32]),
    )
    NDArray1DFloat64 = GenericAlias(
        np.ndarray,
        (OneDim, _DType[np.float64]),
    )
    NDArray1DInt32 = GenericAlias(
        np.ndarray,
        (OneDim, _DType[np.int32]),
    )
    NDArray1DInt64 = GenericAlias(
        np.ndarray,
        (OneDim, _DType[np.int64]),
    )
    NDArray1DStr = GenericAlias(
        np.ndarray,
        (OneDim, _DType[np.str_]),
    )
    NDArray1DTimeDelta64 = GenericAlias(
        np.ndarray,
        (OneDim, _DType[np.timedelta64]),
    )
    NDArray1DUInt32 = GenericAlias(
        np.ndarray,
        (OneDim, _DType[np.uint32]),
    )
    NDArray1DUInt8 = GenericAlias(
        np.ndarray,
        (OneDim, _DType[np.uint8]),
    )
    NDArray2DFloat32 = GenericAlias(
        np.ndarray,
        (TwoDims, _DType[np.float32]),
    )
    NDArray2DFloat64 = GenericAlias(
        np.ndarray,
        (TwoDims, _DType[np.float64]),
    )
    NDArray2DInt64 = GenericAlias(
        np.ndarray,
        (TwoDims, _DType[np.int64]),
    )
    NDArray2DStr = GenericAlias(
        np.ndarray,
        (TwoDims, _DType[np.str_]),
    )
    NDArray2DUInt64 = GenericAlias(
        np.ndarray,
        (TwoDims, _DType[np.int64]),
    )
    NDArray2DFloating = GenericAlias(
        np.ndarray,
        (TwoDims, _DType[np.floating[Any]]),
    )
