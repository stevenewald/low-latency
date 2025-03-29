# Low Latency League

Welcome to the low latency league! This is an internal NUFT challenge to see who can make the fastest matching engine. 

# Testing

The premise is simple: optimize the provided matching engine. Your submission must pass all tests (`make test`) to be considered. 

# Benchmarking

Submit your submission for benchmarking via scholes. There is a live leaderboard, but you can choose to remain anonymous.

Benchmarks will call your orderbook functions (add, remove, match, etc). They are designed to somewhat reflect real world conditions. For example, during times of high volatility, we may see XYZ finish this.

# Guidelines
1. Anything *within reason* is allowed. There are no preset memory limits, limited libraries, etc. I trust you to use your judgement. Adding libraries like Abseil is fine, 5G memory usage is fine, but rewriting in Rust and using 64G memory is not.
2. You cannot change function signatures in engine.hpp. Anything else is fine.
3. Please don't try to break things (particularly the leaderboard). Use your judgement
