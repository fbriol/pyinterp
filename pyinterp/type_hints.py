"""Type hints for pyinterp package."""

from typing import TYPE_CHECKING, Any, Literal

if TYPE_CHECKING:
    import numpy as np

    OneDim = tuple[int]
    TwoDims = tuple[int, int]
    ThreeDims = tuple[int, int, int]
    FourDims = tuple[int, int, int, int]

    Vector3Float32 = np.ndarray[tuple[Literal[3]], np.dtype[np.float32]]
    Vector3Float64 = np.ndarray[tuple[Literal[3]], np.dtype[np.float64]]

    NDArray1D = np.ndarray[OneDim, np.dtype[Any]]
    NDArray1DBool = np.ndarray[OneDim, np.dtype[np.int8]]
    NDArray1DDateTime = np.ndarray[OneDim, np.dtype[np.datetime64]]
    NDArray1DDateTime64 = np.ndarray[OneDim, np.dtype[np.datetime64]]
    NDArray1DFloat32 = np.ndarray[OneDim, np.dtype[np.float32]]
    NDArray1DFloat64 = np.ndarray[OneDim, np.dtype[np.float64]]
    NDArray1DInt64 = np.ndarray[OneDim, np.dtype[np.int64]]
    NDArray1DTimeDelta64 = np.ndarray[OneDim, np.dtype[np.timedelta64]]
    NDArray1DUInt32 = np.ndarray[OneDim, np.dtype[np.uint32]]

    NDArray2DFloat32 = np.ndarray[TwoDims, np.dtype[np.float32]]
    NDArray2DFloat64 = np.ndarray[TwoDims, np.dtype[np.float64]]
    NDArray2DInt64 = np.ndarray[TwoDims, np.dtype[np.int64]]
