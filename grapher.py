import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import glob
import os

plt.style.use('ggplot')

def cargar_y_etiquetar(patron, tipo_algoritmo):
    archivos = glob.glob(patron)
    dfs = []
    for f in archivos:
        df = pd.read_csv(f)
        nombre = os.path.basename(f)
        # Extraer k y hilos del nombre si no están en columnas
        if 'kway' in nombre or 'complete' in nombre:
            df['k_val'] = int(nombre.split('_')[2])
        if 'threads' in nombre:
            df['p_val'] = int(nombre.split('_')[-1].replace('threads.csv', ''))
        else:
            df['p_val'] = 1
        df['tipo'] = tipo_algoritmo
        dfs.append(df)
    return pd.concat(dfs, ignore_index=True) if dfs else pd.DataFrame()

def plot_k_influencia_sec(df):
    n_max = df['n'].max()
    plt.figure(figsize=(12, 7)) 

    for k in sorted(df['k_val'].unique()):
        subset = df[(df['k_val'] == k)]
        plt.plot(subset['n'], subset['t_mean'], marker='o', label=f'k={k}')
    
    plt.xscale('log', base=2)
    plt.xlabel('n')
    plt.ylabel('Tiempo promedio (ns)')
    plt.title(f'Comparacion de k en complete mergesort secuencial')
    plt.legend(title="k")
    plt.grid(True, which="both", ls="-", alpha=0.5)
    plt.savefig('comparacion_k_secuencial.png')
    plt.show()

def plot_k_influencia_par(df):
    n_max = df['n'].max()
    plt.figure(figsize=(12, 7))
    
    for k in sorted(df['k_val'].unique()):
        subset = df[(df['k_val'] == k) & (df['n'] == n_max)].sort_values('p_val')
        plt.plot(subset['p_val'], subset['t_mean'], marker='o', label=f'k={k}')
    
    plt.xlabel('Número de hilos (p)')
    plt.ylabel('Tiempo promedio (ns)')
    plt.title(f'Comparación de k en K-Way Mergesort Paralelo (n=2^{int(np.log2(n_max))})')
    plt.legend(title="Valor de k")
    plt.grid(True, which="both", ls="-", alpha=0.5)
    plt.savefig('k_influencia_paralelo.png')
    plt.show()

def plot_k_influencia_par_matriz(df):
    n_vals = sorted(df['n'].unique())
    
    ncols = 2  
    nrows = 2 
    
    fig, axes = plt.subplots(nrows, ncols, figsize=(12, 10), squeeze=False)
    plt.title('Comparación de k en k-way mergesort paralelo por tamaño n', fontsize=14, fontweight='bold')
    
    axes = axes.flatten()
    
    lines = []
    labels = []

    for i, n_val in enumerate(n_vals):
        if i >= len(axes): break 
        
        ax = axes[i]
        n_pow = int(np.log2(n_val))
        
        for k in sorted(df['k_val'].unique()):
            subset = df[(df['k_val'] == k) & (df['n'] == n_val)].sort_values('p_val')
            if not subset.empty:
                line, = ax.plot(subset['p_val'], subset['t_mean'], marker='o', 
                                 markersize=4, linewidth=1.5)
                if i == 0:
                    lines.append(line)
                    labels.append(f'k = {k}')
        
        ax.set_title(f'n = 2^{n_pow}', fontsize=11, fontweight='bold')
        ax.tick_params(axis='both', which='major', labelsize=9)
        ax.set_xlabel('hilos (p)', fontsize=10)
        ax.set_ylabel('Tiempo promedio (ns)', fontsize=10)
        ax.grid(True, alpha=0.4, linestyle='--')

    for j in range(i + 1, len(axes)):
        fig.delaxes(axes[j])

    fig.legend(lines, labels, loc='upper center', ncol=len(labels), 
               fontsize=11, bbox_to_anchor=(0.5, 1.02), frameon=True, shadow=True)

    plt.tight_layout(rect=[0, 0, 1, 0.97]) 
    plt.savefig('comparacion_k_paralelo.png', dpi=300, bbox_inches='tight')
    plt.show()

def calcular_metricas(df_par, df_sec):
    on_cols = ['n']
    if 'k' in df_par.columns and 'k' in df_sec.columns:
        on_cols.append('k')
    
    merged = df_par.merge(df_sec[['n', 't_mean'] + (['k'] if 'k' in on_cols else [])], 
                         on=on_cols, suffixes=('_p', '_1'))
    
    merged['speedup'] = merged['t_mean_1'] / merged['t_mean_p']

    merged['eficiencia'] = merged['speedup'] / merged['threads']
    
    return merged

def plot_evaluacion_experimental(df, titulo_prefix, filename_prefix):
    n_max = df['n'].max()
    n_pow = int(np.log2(n_max))
    df_n = df[df['n'] == n_max]
    
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(16, 6))
    
    for label, group in (df_n.groupby('k') if 'k' in df_n.columns else [('Normal', df_n)]):
        group = group.sort_values('threads')
        ax1.plot(group['threads'], group['speedup'], marker='o', label=f'k={label}' if 'k' in df_n.columns else 'Mergesort')
    
    p_vals = sorted(df_n['threads'].unique())
    ax1.plot(p_vals, p_vals, 'k--', alpha=0.7, label='Ideal (Lineal)')
    
    ax1.set_title(f'Speedup vs Hilos (n=2^{n_pow})')
    ax1.set_xlabel('hilos (p)')
    ax1.set_ylabel('Speedup $T_1/T_p$')
    ax1.legend()
    ax1.grid(True, alpha=0.3)

    for label, group in (df_n.groupby('k') if 'k' in df_n.columns else [('Normal', df_n)]):
        group = group.sort_values('threads')
        ax2.plot(group['threads'], group['eficiencia'], marker='s', label=f'k={label}' if 'k' in df_n.columns else 'Mergesort')
    
    ax2.axhline(y=1.0, color='r', linestyle='--', alpha=0.5, label='Eficiencia 1.0')
    ax2.set_title(f'Eficiencia vs Hilos (n=2^{n_pow})')
    ax2.set_xlabel('hilos (p)')
    ax2.set_ylabel('Eficiencia $S/p$')
    ax2.set_ylim(0, 1.1)
    ax2.legend()
    ax2.grid(True, alpha=0.3)

    plt.suptitle(f'Evaluación Experimental: {titulo_prefix}', fontsize=14)
    plt.tight_layout(rect=[0, 0.03, 1, 0.95])
    plt.savefig(f'{filename_prefix}_metricas.png', dpi=300)
    plt.show()


# df_kway_sec = cargar_y_etiquetar('results/results_kway_*_sec.csv', 'Secuencial')
# df_kway_par = cargar_y_etiquetar('results/results_kway_*_par_*threads.csv', 'Paralelo')

# plot_k_influencia_sec(df_kway_sec)
# # plot_k_influencia_par(df_kway_par)
# plot_k_influencia_par_matriz(df_kway_par)

    
# df_normal_sec = cargar_y_etiquetar('results/results_normal_sec.csv', 'Secuencial')
# df_normal_par = cargar_y_etiquetar('results/results_normal_par_*threads.csv', 'Paralelo')
# df_normal_metrica = calcular_metricas(df_normal_par, df_normal_sec)
# plot_evaluacion_experimental(df_normal_metrica, "Mergesort Clásico", "normal")

# df_kway_metrica = calcular_metricas(df_kway_par, df_kway_sec)
# plot_evaluacion_experimental(df_kway_metrica, "K-Way Mergesort", "kway")

df_complete_sec = cargar_y_etiquetar('results/results_complete_*_sec.csv', 'Secuencial')
df_complete_par = cargar_y_etiquetar('results/results_complete_*_par_*threads.csv', 'Paralelo')
df_complete_metrica = calcular_metricas(df_complete_par, df_complete_sec)

plot_k_influencia_sec(df_complete_sec)
plot_k_influencia_par_matriz(df_complete_par)
plot_evaluacion_experimental(df_complete_metrica, "Complete Mergesort", "complete")
