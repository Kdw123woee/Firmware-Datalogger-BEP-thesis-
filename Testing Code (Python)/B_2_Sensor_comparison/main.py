import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
import scipy.integrate as si


mu = chr(956)
deg = chr(176)

def convertdata(data):
    # extract voltages and time points from csv
    volts = np.asarray(data.iloc[:,3].values)
    time = np.asarray(data.iloc[:,2].values)
    sample_interval = float(data.iloc[0,:].values[1]) # read sample interval from csv
    R = 467
    amps = volts / R  # amps in A
    amps *= 1000000  #  amps in uA

    # uses sample interval to convert linspace to actual timestamps starting at 0ms
    # very rudimentary way to find the start:
    # find the first value where amps > 100 (arbitrary high enough)
    # find the last value where amps > 100
    # apply some offset
    a = np.where(amps > 80)[0][0]
    b = np.where(amps > 80)[0][-1]
    amps = amps[a-10:b+50]
    time = np.linspace(0, len(amps) * sample_interval, len(amps))
    time *= 1000 # convert to ms
    return amps, time


def calc_energy(amps, time):
    watts = amps * 2
    energy = si.simpson(y=watts, x=time/1000)
    return watts, energy


if __name__ == "__main__":
    # read oscilloscope csv into pandas databuffer
    data_AT = pd.read_csv('AT_47k.CSV', usecols=[0,1,3,4])
    data_TI = pd.read_csv('TI_47k.CSV', usecols=[0,1,3,4])

    # convert voltage/time data to current/time, normalise start of active cycle at 0ms
    amps_AT, time_AT = convertdata(data_AT)
    watts_AT, energy_AT = calc_energy(amps_AT, time_AT)
    print('energy AT: ', energy_AT)
    print("max power AT: ", max(watts_AT))

    amps_TI, time_TI = convertdata(data_TI)
    watts_TI, energy_TI = calc_energy(amps_TI, time_TI)
    print(" ")
    print("energy TI: ", energy_TI)
    print("max power TI: ", max(watts_TI))

    # plot both sensor graphs
    plt.figure(1)
    plt.plot(time_AT, amps_AT, color='#00A6D6')
    plt.plot(time_TI, amps_TI, color='#E03C31')
    plt.ylabel(f"Current ({mu}A)")
    plt.xlabel("Time (ms)")
    plt.title("Active cycle current draw for different sensors")
    plt.legend(["AT", "TI"])

    plt.show()
