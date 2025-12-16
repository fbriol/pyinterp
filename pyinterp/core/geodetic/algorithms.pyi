import enum

from . import Spheroid, _Concept

ANDOYER: Strategy
KARNEY: Strategy
THOMAS: Strategy
VINCENTY: Strategy

def area(geometry: _Concept, wgs: Spheroid | None = None, strategy: Strategy = VINCENTY) -> float:
    ...

class Strategy(enum.Enum):
    ANDOYER  = ...
    KARNEY  = ...
    THOMAS  = ...
    VINCENTY  = ...
