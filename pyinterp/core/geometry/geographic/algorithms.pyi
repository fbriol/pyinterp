from typing import overload
import enum

from . import (
    _Concept,
    Box,
    LineString,
    MultiLineString,
    MultiPoint,
    MultiPolygon,
    Point,
    Polygon,
    Segment,
    Spheroid,
)

ANDOYER: Strategy
KARNEY: Strategy
THOMAS: Strategy
VINCENTY: Strategy

def area(
    geometry: _Concept,
    spheroid: Spheroid | None = None,
    strategy: Strategy = VINCENTY,
) -> float: ...
def azimuth(
    point1: Point,
    point2: Point,
    spheroid: Spheroid | None = None,
    strategy: Strategy = VINCENTY,
) -> float: ...
def centroid(geometry: Box | Segment) -> Point: ...
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
    spheroid: Spheroid | None = None,
    strategy: Strategy = VINCENTY,
) -> Segment: ...
def is_empty(geometry: _Concept) -> bool: ...
def is_simple(geometry: _Concept) -> bool: ...
@overload
def is_valid(geometry: _Concept, return_reason: bool) -> tuple[bool, str]: ...
@overload
def is_valid(geometry: _Concept) -> bool: ...

class Strategy(enum.Enum):
    ANDOYER = ...
    KARNEY = ...
    THOMAS = ...
    VINCENTY = ...
