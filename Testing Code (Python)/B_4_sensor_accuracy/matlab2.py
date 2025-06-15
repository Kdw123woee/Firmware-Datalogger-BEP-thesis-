import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
import scipy.io as io

deg = chr(176)


if __name__ == "__main__":

    ref1 = io.loadmat("Ref_data.mat")
    ref2 = io.loadmat("T2.mat")
    ti1 = io.loadmat("TI_data.mat")
    ti2 = io.loadmat("num2.mat")

    reftemp1 = np.asarray(ref1['T'][0])
    reftemp2 = np.asarray(ref2['T'][0])[1400:]
    titemp1 = np.asarray(ti1['num'][0])
    titemp2 = np.asarray(ti2['num'][0])[1400:]
    combinedref = np.hstack((reftemp1, reftemp2))
    combinedti = np.hstack((titemp1, titemp2))

    # delete samples to only keep the ones when oven stable
    samples_to_keep = 400
    samples_to_delete = 1800 - samples_to_keep + 1
    sample_delete = np.arange(0, samples_to_delete, 1)
    for i in range(5):
        i += 1
        index = 1800*i
        sample_delete = np.append(sample_delete,
                                  np.arange(index, index+samples_to_delete, 1))

    combinedref = np.delete(combinedref, sample_delete)
    combinedti = np.delete(combinedti, sample_delete)

    sample_delete = np.arange(300, 401, 1)
    for i in range(5):
        i += 1
        index = 400 * i
        sample_delete = np.append(sample_delete,
                                  np.arange(index-100, index+1, 1))

    combinedref = np.delete(combinedref, sample_delete)
    combinedti = np.delete(combinedti, sample_delete)

    # calculate difference and statistics
    diff = abs(combinedti - combinedref)
    mean_diff = np.mean(diff)
    std_diff = np.std(diff)
    max_diff = np.max(diff)
    min_diff = np.min(diff)

    # display nicely in pd dataframe
    a = np.column_stack((mean_diff, std_diff, max_diff, min_diff))
    df = pd.DataFrame(a, columns=['mean diff', 'std',
                                  'max diff', 'min diff'])
    print(df.to_string(index=False))

    plt.figure(1)
    plt.plot(combinedref, color="#00A6D6")
    plt.plot(combinedti, color='#E03C31')
    plt.ylabel(f"Temperature ({deg}C)")
    plt.xlabel("Samples")
    plt.title("Measured temperature")
    plt.legend(["Reference", "TMP75B"])

    plt.figure(2)
    plt.plot(diff, color='#00A6D6')
    plt.ylabel(f"Temperature ({deg}C)")
    plt.xlabel("Samples")
    plt.title("Absolute difference in temperature")

    plt.show()

