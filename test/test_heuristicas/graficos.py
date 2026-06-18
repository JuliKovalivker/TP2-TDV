import pandas as pd
import matplotlib.pyplot as plt
import sys

# --- Cargar datos ---
csv_path = sys.argv[1] if len(sys.argv) > 1 else "resultados_e.csv"
df = pd.read_csv(csv_path)

# --- Agrupar costos por heurística ---
heuristicas = sorted(df["heuristica"].unique())
datos = [df[df["heuristica"] == h]["costo"].values for h in heuristicas]

# --- Graficar ---
fig, ax = plt.subplots(figsize=(8, 6))

bp = ax.boxplot(
    datos,
    labels=heuristicas,
    patch_artist=True,
    medianprops=dict(color="black", linewidth=2),
)

colors = ["#4C72B0", "#DD8452", "#55A868"]
for patch, color in zip(bp["boxes"], colors):
    patch.set_facecolor(color)
    patch.set_alpha(0.7)

ax.set_xlabel("Heurística", fontsize=13)
ax.set_ylabel("Costo", fontsize=13)
ax.set_title("Distribución de costos por heurística (instancias gap_e)", fontsize=15, fontweight="bold")
ax.yaxis.grid(True, linestyle="--", alpha=0.7)
ax.set_axisbelow(True)

plt.tight_layout()
plt.savefig("boxplot_heuristicas.png", dpi=150)
plt.show()
print("Gráfico guardado como boxplot_heuristicas.png")