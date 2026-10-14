import csv, sys
from pathlib import Path
p=Path(sys.argv[1] if len(sys.argv)>1 else 'output.csv')
expected=['route','week_of','cost_per_tonne_km','vs_own_history','vs_similar_routes','flagged','matched_note_id','reason']
with p.open(newline='',encoding='utf-8') as f:
    reader=csv.DictReader(f); rows=list(reader)
assert reader.fieldnames == expected, (reader.fieldnames, expected)
assert len(rows) == 728, f'expected 728 route-week rows, got {len(rows)}'
for i,r in enumerate(rows, start=2):
    assert r['route'] and r['week_of'], f'missing route/week at row {i}'
    assert float(r['cost_per_tonne_km']) > 0, f'invalid cost at row {i}'
    if r['flagged']=='No (justified)':
        assert r['matched_note_id'].startswith('N'), f'justified row missing note ID at row {i}'
        assert r['matched_note_id'] in r['reason'], f'note ID not in reason at row {i}'
    if r['flagged']=='Yes':
        assert not r['matched_note_id'], f'unexplained flag unexpectedly cites note at row {i}'
print(f'PASS: schema, {len(rows)} rows, positive costs, and note-citation sanity checks.')
