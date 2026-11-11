#!/bin/bash
# =============================================================================
# Script de experimentación genérico — Tarea 1 Sistemas Distribuidos y Paralelos
# Uso: ./run_experiments.sh <ejecutable> [opciones]
# =============================================================================

# ─── Colores para output ─────────────────────────────────────────────────────
RED='\033[0;31m'; GREEN='\033[0;32m'; YELLOW='\033[1;33m'
CYAN='\033[0;36m'; BOLD='\033[1m'; NC='\033[0m'

# ─── Configuración por defecto ────────────────────────────────────────────────
EXECUTABLE=""
OUTPUT_DIR="results"
REPETITIONS=5
SIZES=(20 22 24 26)        # exponentes: n = 2^e
THREADS=(1 2 4 8)          # cantidad de threads OMP
MODE="parallel"            # "sequential" | "parallel" | "both"
EXTRA_ARGS=""              # argumentos extra para el ejecutable (ej: -k 4)

# ─── Ayuda ────────────────────────────────────────────────────────────────────
usage() {
    echo -e "${BOLD}Uso:${NC}"
    echo "  $0 -e <ejecutable> [opciones]"
    echo ""
    echo -e "${BOLD}Opciones:${NC}"
    echo "  -e <ejecutable>     Ruta al binario compilado (requerido)"
    echo "  -o <dir>            Directorio de salida (default: results)"
    echo "  -r <n>              Repeticiones por experimento (default: 5)"
    echo "  -s <e1,e2,...>      Exponentes de tamaño 2^e (default: 20,22,24,26)"
    echo "  -t <p1,p2,...>      Cantidades de threads (default: 1,2,4,8)"
    echo "  -m <modo>           sequential | parallel | both (default: parallel)"
    echo "  -x <args>           Argumentos extra para el ejecutable (ej: \"-k 4\")"
    echo "  -h                  Mostrar esta ayuda"
    echo ""
    echo -e "${BOLD}Ejemplos:${NC}"
    echo "  # Mergesort paralelo (tu parte):"
    echo "  $0 -e ./ms -m both"
    echo ""
    echo "  # K-way mergesort con k=4:"
    echo "  $0 -e ./kms -m both -x \"-k 4\""
    echo ""
    echo "  # Solo secuencial con más repeticiones:"
    echo "  $0 -e ./ms_seq -m sequential -r 10"
    exit 0
}

# ─── Parsear argumentos ───────────────────────────────────────────────────────
while getopts "e:o:r:s:t:m:x:h" opt; do
    case $opt in
        e) EXECUTABLE="$OPTARG" ;;
        o) OUTPUT_DIR="$OPTARG" ;;
        r) REPETITIONS="$OPTARG" ;;
        s) IFS=',' read -ra SIZES <<< "$OPTARG" ;;
        t) IFS=',' read -ra THREADS <<< "$OPTARG" ;;
        m) MODE="$OPTARG" ;;
        x) EXTRA_ARGS="$OPTARG" ;;
        h) usage ;;
        *) echo -e "${RED}Opción inválida: -$OPTARG${NC}"; usage ;;
    esac
done

# ─── Validaciones ─────────────────────────────────────────────────────────────
if [ -z "$EXECUTABLE" ]; then
    echo -e "${RED}Error: debes indicar el ejecutable con -e${NC}"
    usage
fi

if [ ! -f "$EXECUTABLE" ]; then
    echo -e "${RED}Error: no se encuentra el ejecutable '$EXECUTABLE'${NC}"
    exit 1
fi

# ─── Setup ────────────────────────────────────────────────────────────────────
mkdir -p "$OUTPUT_DIR"
TIMESTAMP=$(date +"%Y%m%d_%H%M%S")
EXE_NAME=$(basename "$EXECUTABLE")
CSV_FILE="$OUTPUT_DIR/${EXE_NAME}_${TIMESTAMP}.csv"
LOG_FILE="$OUTPUT_DIR/${EXE_NAME}_${TIMESTAMP}.log"

echo -e "${BOLD}${CYAN}╔══════════════════════════════════════════════════╗${NC}"
echo -e "${BOLD}${CYAN}║     Tarea 1 — Experimentos de Ordenamiento       ║${NC}"
echo -e "${BOLD}${CYAN}╚══════════════════════════════════════════════════╝${NC}"
echo -e "  Ejecutable : ${BOLD}$EXECUTABLE${NC}"
echo -e "  Modo       : ${BOLD}$MODE${NC}"
echo -e "  Tamaños    : ${BOLD}${SIZES[*]}${NC} (exponentes de 2)"
echo -e "  Threads    : ${BOLD}${THREADS[*]}${NC}"
echo -e "  Repeticiones: ${BOLD}$REPETITIONS${NC}"
echo -e "  Salida CSV : ${BOLD}$CSV_FILE${NC}"
[ -n "$EXTRA_ARGS" ] && echo -e "  Args extra : ${BOLD}$EXTRA_ARGS${NC}"
echo ""

# ─── Cabecera del CSV ─────────────────────────────────────────────────────────
echo "ejecutable,modo,n_exp,n,threads,rep,tiempo_s,sorted" > "$CSV_FILE"

# ─── Función: ejecutar un experimento y extraer tiempo ────────────────────────
# El ejecutable debe imprimir una línea con: "Time: X.XXXXXX seconds"
# y opcionalmente: "Sorted: Yes"
run_experiment() {
    local exe="$1"
    local n="$2"
    local threads="$3"
    local extra="$4"

    # Setear threads OMP
    export OMP_NUM_THREADS=$threads

    # Ejecutar con hasta 3 reintentos (WSL puede perder output en ejecuciones rápidas)
    local output exit_code attempt
    for attempt in 1 2 3; do
        output=$("$exe" "$n" $extra 2>>"$LOG_FILE")
        exit_code=$?
        [ -n "$output" ] && break
        sleep 0.1
    done

    if [ $exit_code -ne 0 ]; then
        echo "ERROR|ERROR"
        return
    fi

    # Extraer tiempo — acepta cualquier número decimal en la línea con "time"
    local time
    time=$(echo "$output" | grep -i "^time" | grep -oP '[0-9]+\.?[0-9]*([eE][-+]?[0-9]+)?' | head -1)

    # Extraer si está ordenado
    local sorted
    sorted=$(echo "$output" | grep -i "sorted" | grep -oP '(Yes|No)' | head -1)
    sorted=${sorted:-"?"}

    echo "${time:-ERROR}|${sorted}"
}

# ─── Función: calcular promedio de una lista de tiempos ───────────────────────
calc_average() {
    local times=("$@")
    local sum=0
    local count=0
    for t in "${times[@]}"; do
        if [[ "$t" =~ ^[0-9]+\.[0-9]+$ ]]; then
            sum=$(echo "$sum + $t" | bc -l)
            ((count++))
        fi
    done
    if [ $count -eq 0 ]; then echo "ERROR"; return; fi
    echo "scale=6; $sum / $count" | bc -l
}

# ─── Almacenar T1 (secuencial con 1 thread) por tamaño para calcular speedup ──
declare -A T1_SEQ   # T1_SEQ[n_exp] = tiempo promedio secuencial

# ─── Tabla de resultados en pantalla ──────────────────────────────────────────
print_header() {
    printf "\n${BOLD}%-8s %-12s %-8s %-12s %-10s %-10s${NC}\n" \
        "Modo" "n (2^e)" "Threads" "Tiempo prom" "Speedup" "Eficiencia"
    printf '%0.s─' {1..65}; echo
}

# ─── Loop principal ───────────────────────────────────────────────────────────

# --- MODO SECUENCIAL ---
if [[ "$MODE" == "sequential" || "$MODE" == "both" ]]; then
    echo -e "\n${BOLD}${YELLOW}▶ Experimentos secuenciales (OMP_NUM_THREADS=1)${NC}"
    print_header

    for exp in "${SIZES[@]}"; do
        n=$((1 << exp))
        times=()

        for ((rep=1; rep<=REPETITIONS; rep++)); do
            result=$(run_experiment "$EXECUTABLE" "$n" 1 "$EXTRA_ARGS")
            time_val=$(echo "$result" | cut -d'|' -f1)
            sorted=$(echo "$result" | cut -d'|' -f2)
            times+=("$time_val")
            echo "$EXE_NAME,sequential,$exp,$n,1,$rep,$time_val,$sorted" >> "$CSV_FILE"
        done

        avg=$(calc_average "${times[@]}")
        T1_SEQ[$exp]=$avg

        if [[ "$avg" == "ERROR" ]]; then
            printf "${RED}%-8s %-12s %-8s %-12s %-10s %-10s${NC}\n" \
                "seq" "2^$exp=$n" "1" "ERROR" "-" "-"
        else
            printf "${GREEN}%-8s %-12s %-8s %-12.4f %-10s %-10s${NC}\n" \
                "seq" "2^$exp" "1" "$avg" "1.00x" "1.000"
        fi
    done
fi

# --- MODO PARALELO ---
if [[ "$MODE" == "parallel" || "$MODE" == "both" ]]; then
    echo -e "\n${BOLD}${YELLOW}▶ Experimentos paralelos${NC}"
    print_header

    for exp in "${SIZES[@]}"; do
        n=$((1 << exp))

        # Si no tenemos T1 del modo secuencial, medirlo con 1 thread
        if [ -z "${T1_SEQ[$exp]}" ]; then
            echo -e "  ${CYAN}Midiendo T1 para n=2^$exp...${NC}"
            times_t1=()
            for ((rep=1; rep<=REPETITIONS; rep++)); do
                result=$(run_experiment "$EXECUTABLE" "$n" 1 "$EXTRA_ARGS")
                time_val=$(echo "$result" | cut -d'|' -f1)
                sorted=$(echo "$result" | cut -d'|' -f2)
                times_t1+=("$time_val")
                echo "$EXE_NAME,parallel,$exp,$n,1,$rep,$time_val,$sorted" >> "$CSV_FILE"
            done
            T1_SEQ[$exp]=$(calc_average "${times_t1[@]}")
        fi

        t1="${T1_SEQ[$exp]}"

        for p in "${THREADS[@]}"; do
            times=()

            for ((rep=1; rep<=REPETITIONS; rep++)); do
                result=$(run_experiment "$EXECUTABLE" "$n" "$p" "$EXTRA_ARGS")
                time_val=$(echo "$result" | cut -d'|' -f1)
                sorted=$(echo "$result" | cut -d'|' -f2)
                times+=("$time_val")
                echo "$EXE_NAME,parallel,$exp,$n,$p,$rep,$time_val,$sorted" >> "$CSV_FILE"
            done

            avg=$(calc_average "${times[@]}")

            # Calcular speedup y eficiencia
            if [[ "$avg" =~ ^[0-9]+\.[0-9]+$ && "$t1" =~ ^[0-9]+\.[0-9]+$ ]]; then
                speedup=$(echo "scale=3; $t1 / $avg" | bc -l)
                eficiencia=$(echo "scale=3; $speedup / $p" | bc -l)
                printf "${GREEN}%-8s %-12s %-8s %-12.4f %-10s %-10s${NC}\n" \
                    "par" "2^$exp" "$p" "$avg" "${speedup}x" "$eficiencia"
            else
                printf "${RED}%-8s %-12s %-8s %-12s %-10s %-10s${NC}\n" \
                    "par" "2^$exp" "$p" "ERROR" "-" "-"
            fi
        done
        echo ""
    done
fi

# ─── Resumen final ────────────────────────────────────────────────────────────
echo -e "\n${BOLD}${GREEN}✔ Experimentos completados${NC}"
echo -e "  CSV con todos los datos: ${BOLD}$CSV_FILE${NC}"
echo -e "  Log de errores:          ${BOLD}$LOG_FILE${NC}"
echo ""
echo -e "${CYAN}Para graficar los resultados, ejecuta:${NC}"
echo -e "  python3 plot_results.py $CSV_FILE"