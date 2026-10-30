"""Local native-body exports must stay outside every Git checkout.

Factual hashes, source locators and behavioral reports can remain in the mod
repository. Historical RE exporters use this directory for recovered bodies
and declarations; ignored repository paths do not satisfy the source boundary.
"""
from pathlib import Path


def external_directory(relative):
    base = (Path(__file__).resolve().parents[2].parent /
            'tools' / 'native_evidence').resolve()
    target = (base / relative).resolve()
    if not target.is_relative_to(base):
        raise ValueError('Native evidence must stay inside its analysis directory')
    if any((parent / '.git').exists() for parent in (target, *target.parents)):
        raise RuntimeError('Native evidence cannot be written inside a Git tree')
    target.mkdir(parents=True, exist_ok=True)
    return target
