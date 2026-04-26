enum class WeightMode {
    Original,                  // keep weights from file
    UniformRandomDistribution, // random weights in [0, 1]
    Exponential,               // exponential(λ)
    Uniform                  // all weights = 1 
};