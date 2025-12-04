from typing import TYPE_CHECKING, Any

if TYPE_CHECKING:

    import numpy as np

    OneDim = tuple[int]
    TwoDims = tuple[int, int]
    ThreeDims = tuple[int, int, int]
    FourDims = tuple[int, int, int, int]

    NDArray1D = np.ndarray[OneDim, np.dtype[Any]]
    NDArray1DBool = np.ndarray[OneDim, np.dtype[np.int8]]
    NDArray1DDateTime = np.ndarray[OneDim, np.dtype[np.datetime64]]
    NDArray1DDateTime64 = np.ndarray[OneDim, np.dtype[np.datetime64]]
    NDArray1DFloat32 = np.ndarray[OneDim, np.dtype[np.float32]]
    NDArray1DFloat64 = np.ndarray[OneDim, np.dtype[np.float64]]
    NDArray1DInt64 = np.ndarray[OneDim, np.dtype[np.int64]]
    NDArray1DTimeDelta64 = np.ndarray[OneDim, np.dtype[np.timedelta64]]

    NDArray2DInt64 = np.ndarray[TwoDims, np.dtype[np.int64]]
