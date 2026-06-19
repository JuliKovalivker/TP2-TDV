# gap_simulator

Simulador para el **Problema de Asignación Generalizada (GAP)**, que asigna vendedores a depósitos minimizando el costo total.

---

## Compilación

Desde el directorio `src/`:

```bash
cd src
make
```

---

## Ejecución

```bash
./gap_simulator [archivo_instancia] [--heuristica-N]
```

### Argumentos

| Argumento | Descripción |
|---|---|
| `archivo_instancia` | Ruta al archivo de instancia. Por defecto: `instances/real/real_instance` |
| `--heuristica-1` | Heurística basada en **varianzas** |
| `--heuristica-2` | Heurística basada en **depósitos** |
| `--heuristica-3` | Heurística basada en **demandas** |

> Es obligatorio especificar exactamente una heurística.

---

## Ejemplos

Ejecutar con la instancia por defecto usando la heurística de varianzas:

```bash
./gap_simulator --heuristica-1
```

Ejecutar con una instancia personalizada usando la heurística de depósitos:

```bash
./gap_simulator instances/test/mi_instancia --heuristica-2
```

---

## Salida

El programa imprime por consola:

- El archivo de instancia leído
- Cantidad de depósitos (`m`) y vendedores (`n`)
- La asignación resultante
- El costo antes y después de la optimización local

Además, guarda la asignación final en un archivo mediante `guardar_en_archivo()`.

---

## Formato del archivo de instancia

El archivo de instancia debe ser compatible con el formato esperado por `GAPInstance`. Referirse a los archivos en `instances/` como ejemplo.

---

## Estructura del proyecto

```
src/
├── gap_simulator.cpp       # Punto de entrada
└── classes/
    ├── GAPInstance.h       # Lectura y representación de la instancia
    ├── Asignacion.h        # Estructura de la asignación
    └── Solver.h            # Lógica de resolución y optimización
instances/
└── real/
    └── real_instance       # Instancia por defecto
```