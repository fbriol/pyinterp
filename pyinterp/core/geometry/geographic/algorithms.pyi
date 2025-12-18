from typing import overload
import enum

from . import Box, Point, Segment, Spheroid, _Concept

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
    geometry1: _Concept,
    geometry2: _Concept,
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
