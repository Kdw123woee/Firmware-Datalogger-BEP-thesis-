import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

mu = chr(956)
Ohm = chr(937)
approx = chr(8773)

if __name__ == '__main__':
    V = 2
    R = np.arange(4, 200, step=1)  # R in kOhm
    I = V / R  # I in mA
    I *= 1000  # I in uA

    I_diff = np.gradient(I, R)
    print(I_diff[40:50])
    print(R[40])

    # tangent line for visualisation
    x = np.arange(15, 75, step=1)
    y = (-1 * x) + 89

    plt.figure(1)
    plt.plot(R, I, color="#00A6D6")
    plt.plot(x, y, color='#E03C31')
    plt.ylabel(f"Current ({mu}A)")
    plt.xlabel(f"Resistance (k{Ohm})")
    plt.title("Expected current at 2V for different pull-up values")
    plt.legend(["Current", f"Crossover point, dx/dy {approx} -1"])
    plt.show()


