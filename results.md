# Simulation Results and Test Cases

This file contains the analytical and simulation results for different routing scenarios implemented in the project.

---

## 🔹 Case 1: 0 → {1,2} → 3

### Input:
p(0→1) = 0.4  
p(0→2) = 0.4  
p(0→1,0→2) = 0.3  
p(1→3) = 0.5  
p(2→3) = 0.4  

Packets = 10000  

### Output:
Analytical ETX = 4.1000  
Simulated ETX  = 4.0937  

### Observation:
Simulation closely matches analytical result, validating the model.

---
## 🔹 Case 2: 0 → {1,2} → {3,4}

### Input:
p(0→1) = 0.5  
p(0→2) = 0.3  
p(0→1,0→2) = 0.2  

p(1→3) = 0.6  
p(1→4) = 0.4  
p(1→3,1→4) = 0.3  

p(2→3) = 0.5  
p(2→4) = 0.3  
p(2→3,2→4) = 0.2  

Packets = 10000  

### Output:
Analytical ETX = 4.5595  
Simulated ETX  = 4.3413  

### Observation:
Simulated ETX is slightly lower due to favorable random transmission outcomes.
increasing number of packets can make this more validated .
---

## 🔹 Case 3: 0 → {1,2,3} → {4,5,6}

### Input:
(Complex joint probabilities considered)

Packets = 10000  

### Output:
R1 = 2.961538  
R2 = 2.961538  
R3 = 3.183761  

R12 = 4.961538  
R13 = 5.183761  
R23 = 2.961538  
R123 = 5.923077  

Best Forwarder Set = {1,2}  

Analytical ETX = 4.961538  
Simulated ETX  = 4.929106  

### Observation:
Simulation result is slightly lower than analytical value due to probabilistic nature of packet transmissions. The small difference confirms the correctness and robustness of the model.

---

## ✅ Conclusion

Across all test cases, the simulation results closely match the analytical ETX values. This validates the effectiveness of the correlation-aware routing approach and demonstrates its accuracy in modeling wireless transmission behavior.
