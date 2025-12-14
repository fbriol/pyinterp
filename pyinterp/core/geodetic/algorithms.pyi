import enum
from typing import ClassVar, overload

from . import Point, Spheroid, Box, Ring, LineString, Polygon, MultiPolygon

ANDOYER: Strategy
KARNEY: Strategy
THOMAS: Strategy
VINCENTY: Strategy

@overload
def area(geometry: Point, wgs: Spheroid | None = None, strategy: Strategy = VINCENTY) -> float:
    ...
@overload
def area(geometry: Box, wgs: Spheroid | None = None, strategy: Strategy = VINCENTY) -> float:
    ...
@overload
def area(geometry: Ring, wgs: Spheroid | None = None, strategy: Strategy = VINCENTY) -> float:
    ...
@overload
def area(geometry: LineString, wgs: Spheroid | None = None, strategy: Strategy = VINCENTY) -> float:
    ...
@overload
def area(geometry: Polygon, wgs: Spheroid | None = None, strategy: Strategy = VINCENTY) -> float:
    ...
@overload
def area(geometry: MultiPolygon, wgs: Spheroid | None = None, strategy: Strategy = VINCENTY) -> float:
    ...

class Strategy(enum.Enum):
    ANDOYER  = ...
    KARNEY  = ...
    THOMAS  = ...
    VINCENTY  = ...
