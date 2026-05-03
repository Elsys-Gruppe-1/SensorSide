import matplotlib.pyplot as plt
import numpy as np

# --- Konfigurasjon ---
tid = np.arange(1, 11, 1)

# Fargedefinisjoner per dybde
farge_05 = '#2ca02c' # Grønn
farge_10 = '#ff7f0e' # Oransje
farge_15 = '#d62728' # Rød

# --- Data ---
temp_05 = np.array([18.4, 18.7, 18.4, 18.5, 18.6, 18.4, 18.5, 18.6, 18.5, 18.4])
temp_10 = np.array([17.9, 17.4, 17.3, 17.1, 17.2, 17.4, 17.3, 17.2, 17.1, 17.2])
temp_15 = np.array([16.3, 16.0, 15.9, 15.8, 16.1, 16.2, 15.9, 16.0, 15.8, 15.9])

tds_05 = np.array([132, 138, 130, 135, 128, 133, 140, 131, 129, 134])
tds_10 = np.array([510, 525, 490, 505, 515, 480, 530, 495, 500, 510])
tds_15 = np.array([1250, 1380, 1210, 1420, 1290, 1310, 1450, 1260, 1330, 1410])

# Referanseverdier
ref_temp = [18.5, 17.5, 16.2] 
ref_tds = [120, 450, 1200]    

def hent_stats(maalt, ref):
    avvik_per_punkt = np.abs((maalt - ref) / ref) * 100
    snitt_avvik = np.mean(avvik_per_punkt)
    maks_avvik = np.max(avvik_per_punkt)
    return snitt_avvik, maks_avvik

# --- Plotting ---
fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(12, 11), sharex=True)

# --- Plot 1: Temperatur ---
for data, ref, farge, label in zip([temp_05, temp_10, temp_15], ref_temp, [farge_05, farge_10, farge_15], ['0.5m', '1.0m', '1.5m']):
    snitt_av, maks_av = hent_stats(data, ref)
    ax1.axhline(ref, color=farge, linestyle='-', linewidth=1, alpha=0.3)
    ax1.text(1, ref + 0.05, f'Ref: {ref}°C', color=farge, fontsize=8, fontweight='bold', alpha=0.7)
    ax1.plot(tid, data, 'o-', color=farge, 
             label=f'{label} (Snitt avvik: {snitt_av:.1f}%, Maks: {maks_av:.1f}%)')

ax1.set_ylabel('Temperatur [$^\circ$C]')
ax1.set_title('Verifisering av Temperaturmålinger (Digital DS18S20)')
ax1.grid(True, alpha=0.2)
ax1.legend(title="Dybde & Nøyaktighet", loc='center left', bbox_to_anchor=(1, 0.5), framealpha=1, edgecolor='black')

# --- Plot 2: TDS ---
for data, ref, farge, label in zip([tds_05, tds_10, tds_15], ref_tds, [farge_05, farge_10, farge_15], ['0.5m', '1.0m', '1.5m']):
    snitt_av, maks_av = hent_stats(data, ref)
    
    # 1. Referanselinje (Faktisk verdi)
    ax2.axhline(ref, color=farge, linestyle='-', linewidth=1, alpha=0.3)
    ax2.text(1, ref + (ref*0.02), f'Ref: {ref} ppm', color=farge, fontsize=8, fontweight='bold', alpha=0.7)
    
    # 2. 15% Grenselinje (Systemkrav)
    grense_15 = ref * 1.15
    ax2.axhline(grense_15, color=farge, linestyle='--', linewidth=0.8, alpha=0.4)
    # Legger til tekst for grensen på slutten av grafen for å unngå kollisjon med Ref-tekst
    ax2.text(9.2, grense_15 + (ref*0.01), 'Grense 15%', color=farge, fontsize=7, style='italic', alpha=0.5)
    
    # 3. Målepunkter
    ax2.plot(tid, data, 's-', color=farge, 
             label=f'{label} (Snitt avvik: {snitt_av:.1f}%, Maks: {maks_av:.1f}%)')

ax2.set_ylabel('TDS [ppm]')
ax2.set_xlabel('Måleserie nr.')
ax2.set_title('Verifisering av TDS-målinger (Analog Keystudio V1.0)')
ax2.grid(True, alpha=0.2)
ax2.legend(title="Dybde & Nøyaktighet", loc='center left', bbox_to_anchor=(1, 0.5), framealpha=1, edgecolor='black')

plt.tight_layout()
plt.subplots_adjust(right=0.70) # Justert for å gi plass til utvidet legend-tekst

plt.savefig('sensor_verifisering_komplett.png', dpi=300)
plt.show()