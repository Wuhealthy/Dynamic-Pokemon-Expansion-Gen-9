#!/usr/bin/env python3

import re
from pathlib import Path
from build import SPECIES_ABILITY_OVERRIDES


def main():
    path = Path(__file__).resolve().parent.parent / 'src' / 'Base_Stats.c'
    source = path.read_text(encoding='utf-8', errors='replace')

    for species, abilities in SPECIES_ABILITY_OVERRIDES.items():
        block_pattern = re.compile(
            r'(\[' + re.escape(species) + r'\]\s*=\s*\{)(.*?)(?=\n\s*\[SPECIES_|\Z)',
            re.S,
        )
        match = block_pattern.search(source)
        if match is None:
            raise RuntimeError('Species block not found: ' + species)

        block = match.group(2)
        for field, ability in zip(('ability1', 'ability2', 'hiddenAbility'), abilities):
            block, count = re.subn(
                r'(\.' + field + r'\s*=\s*)ABILITY_[A-Z0-9_]+',
                r'\g<1>' + ability,
                block,
                count=1,
            )
            if count != 1:
                raise RuntimeError('%s.%s not found' % (species, field))

        source = source[:match.start(2)] + block + source[match.end(2):]

    path.write_text(source, encoding='utf-8', newline='\n')


if __name__ == '__main__':
    main()
