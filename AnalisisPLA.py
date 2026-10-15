import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import glob
import matplotlib.patches as patches

# =====================================================================
# 1. CONSTANTES FÍSICAS Y DEL MATERIAL (PLA)
# =====================================================================
DENSIDAD_PLA = 1.24  # g/cm^3 (PLA)
MEV_A_JOULES = 1.60218e-13

# Para PLA puro, la escisión de cadena (rotura) domina sobre el cross-linking
G_SCISSION = 0.8  # Roturas de cadena por cada 100 eV absorbidos

# Dimensiones del bloque en la simulación
ANCHO_X = 10.0  # cm
ALTO_Y = 10.0   # cm
LIMITE_Z = (-5.00, 5.00)

# =====================================================================
# 2. CARGA Y FUSIÓN DE DATOS MULTIHILO
# =====================================================================
archivos_csv = glob.glob('datos_dosis_nt_Dosis_t*.csv')
if not archivos_csv:
    raise FileNotFoundError("No se encontraron archivos CSV. Asegúrate de ejecutar Geant4 primero.")

lista_dfs = [pd.read_csv(f, comment='#', names=['X', 'Y', 'Z', 'Edep']) for f in archivos_csv]
df = pd.concat(lista_dfs, ignore_index=True)

# =====================================================================
# 3. PROCESAMIENTO 1D (PERFIL DE PROFUNDIDAD)
# =====================================================================
grosor_capa_1d = 0.1  # Capas de 1 mm (0.1 cm)
bins_z_1d = np.arange(LIMITE_Z[0], LIMITE_Z[1] + grosor_capa_1d, grosor_capa_1d)

# Cálculo de la masa de cada capa para obtener la dosis
volumen_capa_cm3 = ANCHO_X * ALTO_Y * grosor_capa_1d
masa_capa_kg = (volumen_capa_cm3 * DENSIDAD_PLA) / 1000.0  # Convertir gramos a kg

# Agrupar energía
df['Capa_Z'] = pd.cut(df['Z'], bins=bins_z_1d)
energia_total_mev_1d = df.groupby('Capa_Z', observed=False)['Edep'].sum().values
profundidad_centro_1d = bins_z_1d[:-1] + (grosor_capa_1d / 2)

# Conversión a Grays (Gy) = Joules / kg
dosis_gy_1d = (energia_total_mev_1d * MEV_A_JOULES) / masa_capa_kg

# Cálculo de Degradación (Chain Scission)
# Regla: (Energía en eV / 100) * Valor G
energia_ev_1d = energia_total_mev_1d * 1e6
escisiones_totales = (energia_ev_1d / 100.0) * G_SCISSION
densidad_escision = escisiones_totales / volumen_capa_cm3  # Roturas por cm^3

# Inferencia Mecánica: Definir umbral crítico de daño (top 20% de la dosis máxima)
umbral_degradacion = np.max(dosis_gy_1d) * 0.8
indices_criticos = np.where(dosis_gy_1d >= umbral_degradacion)[0]
inicio_critico = profundidad_centro_1d[indices_criticos[0]] if len(indices_criticos) > 0 else None
fin_critico = profundidad_centro_1d[indices_criticos[-1]] if len(indices_criticos) > 0 else None

# =====================================================================
# 4. PROCESAMIENTO 2D (MAPA DE CALOR ESPACIAL)
# =====================================================================
tamaño_celda_2d = 0.2  # Celdas de 2x2 mm en el plano X-Z
bins_x = np.arange(-10.0, 10.0 + tamaño_celda_2d, tamaño_celda_2d)
bins_z_2d = np.arange(LIMITE_Z[0], LIMITE_Z[1] + tamaño_celda_2d, tamaño_celda_2d)

# El volumen de la celda asume la proyección total del eje Y (20 cm)
volumen_celda_2d = tamaño_celda_2d * ALTO_Y * tamaño_celda_2d
masa_celda_2d_kg = (volumen_celda_2d * DENSIDAD_PLA) / 1000.0

heatmap_mev, _, _ = np.histogram2d(df['X'], df['Z'], bins=[bins_x, bins_z_2d], weights=df['Edep'])
heatmap_gy = (heatmap_mev * MEV_A_JOULES) / masa_celda_2d_kg

# =====================================================================
# 5. GENERACIÓN DE LA FIGURA MULTIPANEL
# =====================================================================
fig, axs = plt.subplots(1, 3, figsize=(18, 6))

# Panel 1: Dosis en Grays
axs[0].plot(profundidad_centro_1d, dosis_gy_1d, color='blue', linewidth=2)
axs[0].set_title('Perfil de Dosis Absorbida', fontsize=12)
axs[0].set_xlabel('Profundidad Z (cm)')
axs[0].set_ylabel('Dosis (Grays)')
axs[0].grid(True, linestyle='--', alpha=0.6)

# Panel 2: Escisión de Cadena (Degradación)
axs[1].plot(profundidad_centro_1d, densidad_escision, color='orange', linewidth=2)
if inicio_critico is not None:
    axs[1].axvspan(inicio_critico, fin_critico, color='red', alpha=0.2, label='Zona de Max. Degradación (Pérdida Mecánica)')
    axs[1].legend(loc='upper right')
axs[1].set_title('Degradación Estructural (Chain Scission)', fontsize=12)
axs[1].set_xlabel('Profundidad Z (cm)')
axs[1].set_ylabel('Densidad de Roturas (eventos/cm$^3$)')
axs[1].grid(True, linestyle='--', alpha=0.6)

# Panel 3: Mapa de Calor Espacial 2D en Grays
im = axs[2].imshow(heatmap_gy.T, origin='lower', extent=[-10, 10, -10, 10], cmap='hot', aspect='auto')
cbar = fig.colorbar(im, ax=axs[2])
cbar.set_label('Dosis Absorbida (Grays)')
axs[2].set_title('Mapa Espacial 2D de Daño Estructural', fontsize=12)
axs[2].set_xlabel('Eje X (cm) - Ancho')
axs[2].set_ylabel('Eje Z (cm) - Profundidad')

plt.suptitle(f'Análisis de Propiedades Físico-Mecánicas: Radiación en PLA', fontsize=16, fontweight='bold')
plt.tight_layout()
plt.subplots_adjust(top=0.88)

plt.savefig('analisis_mecanico_PLA.png', dpi=300)
print("Ejecución exitosa. Gráfica consolidada guardada como 'analisis_mecanico_PLA.png'.")
