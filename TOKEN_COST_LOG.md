# Token and cost log

- Input taken full supplied `shipment_records.csv` (2,940 shipment records) plus `context_notes.csv` (10 notes).
- LLM or API calls: 0
- Input tokens: 0
- Output tokens: 0
- Estimated model and API cost: USD $0.00
- Compute one local C++17 executable run no network calls or paid services.
- Reason is  the supplied note set is small and the implementation uses deterministic route and date retrieval and templated explanations rather than generating text with an LLM. This improves auditability and repeatability at the cost of less flexible natural language interpretation.
