import matplotlib.pyplot as plt
import pandas as pd

import numpy as np
import scipy.integrate as si

mu = chr(956)
Ohm = chr(937)


def convertdata(data):
    # extract voltages and time points from csv
    volts = np.asarray(data.iloc[:,3].values)
    time = np.asarray(data.iloc[:,2].values)
    sample_interval = float(data.iloc[0,:].values[1]) # read sample interval from csv
    R = 467
    amps = volts / R  # amps in A
    amps *= 1e6  # amps in uA

    # uses sample interval to convert linspace to actual timestamps starting at 0ms
    # very rudimentary way to find the start:
    # find the first value where amps > 120 (arbitrary high enough)
    # find the last value where amps > 120
    # apply some offset
    a = np.where(amps > 120)[0][0]
    b = np.where(amps > 120)[0][-1]
    amps = amps[a-10:b+50]
    time = np.linspace(0, len(amps) * sample_interval, len(amps))
    time *= 1000  # convert to ms

    return amps, time


def calc_energy(amps, time):
    watts = amps * 2
    energy = si.simpson(y=watts, x=time/1000)
    return watts, energy


if __name__ == "__main__":
    # read oscilloscope csv into pandas databuffer
    data1 = pd.read_csv("4MHz_47k.csv", usecols=[0,1,3,4])
    data2 = pd.read_csv('2MHz_47k.CSV', usecols=[0,1,3,4])
    data3 = pd.read_csv("1MHz_47k.CSV", usecols=[0, 1, 3, 4])
    data4 = pd.read_csv('800kHz_47k.CSV', usecols=[0, 1, 3, 4])
    data5 = pd.read_csv("400kHz_47k.CSV", usecols=[0,1,3,4])

    data1_name = "4MHz"
    data2_name = "2MHz"
    data3_name = "1MHz"
    data4_name = "800kHz"
    data5_name = "400kHz"

    # convert voltage/time data to current/time, normalise start of active cycle at 0ms
    amps1, time1 = convertdata(data1)
    amps2, time2 = convertdata(data2)
    amps3, time3 = convertdata(data3)
    amps4, time4 = convertdata(data4)
    amps5, time5 = convertdata(data5)

    # calculate the power at every time point, and the total energy over the active cycle
    watts1, energy1 = calc_energy(amps1, time1)
    watts2, energy2 = calc_energy(amps2, time2)
    watts3, energy3 = calc_energy(amps3, time3)
    watts4, energy4 = calc_energy(amps4, time4)
    watts5, energy5 = calc_energy(amps5, time5)

    # can be made prettier but does the job just fine
    print(f'energy, peak power {data1_name} : {energy1} ,  {max(watts1)}')
    print(f'energy, peak power {data2_name} : {energy2} ,  {max(watts2)}')
    print(f'energy, peak power {data3_name} : {energy3} ,  {max(watts3)}')
    print(f'energy, peak power {data4_name} : {energy4} ,  {max(watts4)}')
    print(f'energy, peak power {data5_name} : {energy5} ,  {max(watts5)}')

    # plot all graphs at once
    plt.figure(1)
    plt.plot(time1, amps1, color='#0C2340', marker=',')
    plt.plot(time2, amps2, color='#E03C31', marker=',')
    plt.plot(time3, amps3, color='#6CC24A', marker=',')
    plt.plot(time4, amps4, color='#00A6D6', marker=',')
    plt.plot(time5, amps5, color='#FFB81C', marker=',')

    plt.ylabel(f"Current ({mu}A)")
    plt.xlabel("Time (ms)")
    plt.title("Active cycle current draw for different clock frequencies")
    plt.legend(["4MHz", "2MHz","1MHz","800kHz", "400kHz"])

    # plot power of 800kHz, 2V
    plt.figure(3)
    plt.plot(time4, watts4, color='#00A6D6')
    plt.ylabel(f"Power ({mu}W)")
    plt.xlabel("Time (ms)")
    plt.title("Power during MCU active cycle.")

    plt.show()
