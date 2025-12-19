from typing import overload
from .. import geographic
from . import (
    _Concept,
    Box,
    LineString,
    MultiLineString,
    MultiPoint,
    MultiPolygon,
    Point,
    Polygon,
    Ring,
    Segment,
)

def area(
    geometry: _Concept,
) -> float: ...
def azimuth(
    point1: Point,
    point2: Point,
) -> float: ...
def buffer(
    geometry: _Concept,
    distance_strategy: DistanceSymmetric | DistanceAsymmetric,
    join_strategy: JoinRound | JoinMiter,
    end_strategy: EndRound | EndFlat,
    point_strategy: PointCircle | PointSquare,
) -> MultiPolygon: ...
def centroid(geometry: _Concept) -> Point: ...
def clear(geometry: _Concept) -> None: ...
def closest_points(
    geometry1: LineString
    | MultiLineString
    | MultiPoint
    | MultiPolygon
    | Polygon,
    geometry2: LineString
    | MultiLineString
    | MultiPoint
    | MultiPolygon
    | Polygon,
) -> Segment: ...
@overload
def convert_to_geographic(geometry: Point) -> geographic.Point: ...
@overload
def convert_to_geographic(geometry: Segment) -> geographic.Segment: ...
@overload
def convert_to_geographic(geometry: Box) -> geographic.Box: ...
@overload
def convert_to_geographic(geometry: LineString) -> geographic.LineString: ...
@overload
def convert_to_geographic(geometry: Ring) -> geographic.Ring: ...
@overload
def convert_to_geographic(geometry: Polygon) -> geographic.Polygon: ...
@overload
def convert_to_geographic(geometry: MultiPoint) -> geographic.MultiPoint: ...
@overload
def convert_to_geographic(geometry: MultiLineString) -> geographic.MultiLineString: ...
@overload
def convert_to_geographic(geometry: MultiPolygon) -> geographic.MultiPolygon: ...
def convex_hull(geometry: _Concept) -> Polygon: ...
def correct(geometry: _Concept) -> None: ...
def covered_by(
    geometry1: Point
    | Box
    | LineString
    | Ring
    | Polygon
    | MultiPoint
    | MultiLineString
    | MultiPolygon,
    geometry2: Point
    | Segment
    | Box
    | LineString
    | Ring
    | Polygon
    | MultiPoint
    | MultiLineString
    | MultiPolygon,
) -> bool: ...
def crosses(
    geometry1: Point
    | LineString
    | Ring
    | Polygon
    | MultiLineString
    | MultiPolygon,
    geometry2: Point
    | LineString
    | Ring
    | Polygon
    | MultiLineString
    | MultiPolygon,
) -> bool: ...
def densify(geometry: LineString | Ring | Polygon | MultiLineString | MultiPolygon, max_distance: float) -> _Concept: ...
def is_empty(geometry: _Concept) -> bool: ...
def is_simple(geometry: _Concept) -> bool: ...
@overload
def is_valid(geometry: _Concept, return_reason: bool) -> tuple[bool, str]: ...
@overload
def is_valid(geometry: _Concept) -> bool: ...

class DistanceAsymmetric:
    def __init__(self, distance_left: float, distance_right: float) -> None: ...

class DistanceSymmetric:
    def __init__(self, buffer_distance: float) -> None: ...

class EndFlat:
    def __init__(self) -> None: ...

class EndRound:
    def __init__(self, points_per_circle: int = ...) -> None: ...

class JoinMiter:
    def __init__(self, miter_limit: float = ...) -> None: ...

class JoinRound:
    def __init__(self, points_per_circle: int = ...) -> None: ...

class PointCircle:
    def __init__(self, points_per_circle: int = ...) -> None: ...

class PointSquare:
    def __init__(self) -> None: ...
