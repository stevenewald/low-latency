# Low Latency League

Welcome to the Low Latency League — NUFT’s internal challenge to build the fastest matching engine.

## Testing

Your job is simple: optimize the provided engine. Your code **must pass all tests** (`make test`) to qualify.

## Benchmarking

Submit your code for benchmarking through *scholes*. A live leaderboard tracks results — you can choose to stay anonymous.

Benchmarks simulate realistic trading conditions. For example, during high volatility, you may see XYZ spike.

## Guidelines

1. Anything *reasonable* is fair game. There are no strict memory limits or library restrictions. Add Abseil? Sure. Use 5GB RAM? Fine. Rewriting in Rust with 64GB? Not fine.  
2. You **must not** change any function signatures in `engine.hpp`. Everything else is open.  
3. Don’t try to break things (especially the leaderboard). Use common sense.
