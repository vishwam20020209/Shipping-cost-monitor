# Shipping Cost Monitor

A deterministic dependency free implementation of the AI Intern case study. It reads the supplied shipment and context note CSVs and writes `output.csv` using the required eight column schema.

## Things through case study

1. Groups shipments by `origin-destination` route `route_type` and Monday starting week.
2. Calculates weekly cost per tonne km as sum(freight_cost_inr) / sum(quantity_tonnes × distance_km) for all shipments in that route and week group.
3. Compares the current weekly value with the mean of up to the previous eight available weekly values for that route only. No current week data is included in its own baseline. When fewer than eight prior observations exist it uses all available observations.
4. Compares it with the average of all other routes of the same `route_type` in that same week.
5. Flags an increase if it is at least twenty percent above either available baseline. The brief does not specify a numeric anomaly threshold so twenty percent is an explicit configurable policy choice rather than a supplied grading rule.
6. Checks context notes using deterministic route and date rules. Only notes that explicitly describe a cost increasing event can justify an anomaly. A matching note ID and its exact supplied text appear in the reason. Notes describing improved conditions, stable demand, unaffected route or no rate change never justify a spike.

## Important assumptions

- This is a grounded retrieval baseline not a generative LLM or vector database. The provided note set has only ten short notes so explicit deterministic matching is easier to audit and avoids hallucinated explanations. It can be replaced by embeddings or RAG if the note corpus grows while keeping the same strict route validation after retrieval.
- Note matching windows are conservative and explicit:
 N001 is only applicable to Chennai to Bangalore weeks beginning
  Feb 24 or Mar 3 ,2025: N002 applies to Ahmedabad to Mumbai during the seven day festival week beginning
 Jan 20, 2025: N003 applies to all routes for four weeks from May 5, 2025. The four week N003 window is an implementation assumption because the note says the increase starts that week but does not state an end date. It is documented to avoid silently stretching a note indefinitely.
- N004 to N010 do not justify a rise because they state the affected routes are outside the dataset costs were not significantly affected conditions improved or normal demand was stable or rates did not change.
- The twenty percent threshold is a simple conservative heuristic. The case brief does not define the threshold so it should be discussed in the walkthrough and tuned with labeled examples.
- This implementation makes zero LLM calls. For the required full run token cost log input tokens = 0, output tokens = 0, calls = 0 estimated model cost = $0.00. CSV data is processed locally have no external service or API key is needed.
- Costs are emitted to two decimal places in the CSV. Calculations and baselines use full floating point precision before formatting.

## Repository layout

```text
src/main.cpp                 C++17 implementation
 data/shipment_records.csv   supplied shipment input
 data/context_notes.csv      supplied context notes
 data/sample_output_format_v2.csv  supplied output schema and example
 output.csv                  generated results
 run_log.txt                 run summary
 scripts/reproducibility.sh  three run byte for byte check
 tests/evaluate.py           lightweight sanity and evaluation checks
```

## Build and run

Requires a C++17 compiler (g++/clang++).

```bash
g++ -std=c++17 -O2 -Wall -Wextra -pedantic src/main.cpp -o freighttiger
./freighttiger data/shipment_records.csv data/context_notes.csv output.csv
```

Windows PowerShell with MinGW g++:

```powershell
g++ -std=c++17 -O2 -Wall -Wextra -pedantic src/main.cpp -o freighttiger.exe
.\freighttiger.exe data\shipment_records.csv data\context_notes.csv output.csv
```

Optional arguments are shipment CSV path notes CSV path output CSV path.

## Validation performed

- Compiles with C++17 and warning flags.
- Processes the supplied to 940 shipments into 728 route week rows.
- A three run reproducibility script compares the complete output bytes and fails if they differ.
- Sanity checks validate the exact output column order row count numeric costs and that every `No (justified)` row has a note ID while unexplained `Yes` rows do not cite a note.

##  walkthrough the case study

1. Problem framing and input and output contract .
2. Weekly grouping and weighted cost formula .
3. No look ahead rolling history and same week peer baseline .
4. Threshold and conservative note matching non causal notes are rejected .
5. Reproducibility evaluation checks and zero LLM cost trade off .
6. Limitations and next steps labeled through threshold calibration, date aware retrieval tests and optional embeddings or LLM explanations constrained to retrieved notes .
