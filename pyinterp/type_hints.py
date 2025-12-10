"""Type hints for pyinterp package."""

from typing import TYPE_CHECKING, Any, Literal, TypeAlias

if TYPE_CHECKING:
    import numpy as np

    OneDim: TypeAlias = tuple[int]
    TwoDims: TypeAlias = tuple[int, int]
    ThreeDims: TypeAlias = tuple[int, int, int]
    FourDims: TypeAlias = tuple[int, int, int, int]

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
    NDArray1DDateTime: TypeAlias = np.ndarray[
        OneDim,
        np.dtype[np.datetime64],
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
    NDArray1DInt64: TypeAlias = np.ndarray[
        OneDim,
        np.dtype[np.int64],
    ]
    NDArray1DTimeDelta64: TypeAlias = np.ndarray[
        OneDim,
        np.dtype[np.timedelta64],
    ]
    NDArray1DUInt32: TypeAlias = np.ndarray[
        OneDim,
        np.dtype[np.uint32],
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
    NDArray2DUInt64: TypeAlias = np.ndarray[
        TwoDims,
        np.dtype[np.int64],
    ]
