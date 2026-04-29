import matplotlib.pyplot as plt
import numpy as np

# --- Konfigurasjon og Data ---
tid = np.arange(0, 60, 5)

# Faktiske målinger fra systemet
temp_maalt = np.array([20.2, 20.5, 21.0, 21.2, 21.1, 20.9, 20.8, 20.7, 20.6, 20.5, 20.4, 20.3])
tds_maalt = np.array([442, 445, 448, 450, 452, 451, 449, 448, 447, 446, 445, 444])

# Referanseverdier (f.eks. fra kalibrert termometer og kjent væske)
temp_ref = 20.0  # Celsius
tds_ref = 450.0  # ppm

# --- Beregninger ---
def beregn_statistikk(maalt, ref, navn):
    avvik_prosent = np.abs((maalt - ref) / ref) * 100
    maks_avvik = np.max(avvik_prosent)
    snitt = np.mean(maalt)
    print(f"--- {navn} ---")
    print(f"Gjennomsnitt: {snitt:.2f}")
    print(f"Største prosentvise avvik: {maks_avvik:.2f}%")
    return snitt, maks_avvik

snitt_temp, maks_feil_temp = beregn_statistikk(temp_maalt, temp_ref, "Temperatur")
snitt_tds, maks_feil_tds = beregn_statistikk(tds_maalt, tds_ref, "TDS")

# --- Plot 1: Temperatur ---
plt.figure(figsize=(8, 4))
plt.plot(tid, temp_maalt, color='tab:red', marker='o', label='Målt verdi')
plt.axhline(temp_ref, color='green', linestyle='-', label=f'Referanse ({temp_ref}°C)')
plt.axhline(snitt_temp, color='black', linestyle='--', alpha=0.5, label=f'Snitt ({snitt_temp:.2f}°C)')
plt.ylabel('Temperatur [$^\circ$C]')
plt.xlabel('Tid [min]')
plt.title(f'Temperaturverifikasjon (Maks avvik: {maks_feil_temp:.2f}%)')
plt.grid(True, alpha=0.3)
plt.legend()
plt.tight_layout()
plt.savefig('temp_plot.png')
plt.close()

# --- Plot 2: TDS ---
plt.figure(figsize=(8, 4))
plt.plot(tid, tds_maalt, color='tab:blue', marker='s', label='Målt verdi')
plt.axhline(tds_ref, color='green', linestyle='-', label=f'Referanse ({tds_ref} ppm)')
plt.axhline(snitt_tds, color='black', linestyle='--', alpha=0.5, label=f'Snitt ({snitt_tds:.2f} ppm)')
plt.ylabel('TDS [ppm]')
plt.xlabel('Tid [min]')
plt.title(f'TDS-verifikasjon (Maks avvik: {maks_feil_tds:.2f}%)')
plt.grid(True, alpha=0.3)
plt.legend()
plt.tight_layout()
plt.savefig('tds_plot.png')
plt.close()