# Script Gnuplot para gerar os gráficos do Experimento 2
# Salve como plot_script.gp e execute com: gnuplot plot_script.gp

# --- Configurações Gerais ---
set datafile commentschars "#" # Ignora linhas começando com #
set key outside bottom center # Posição da legenda para todos os gráficos
set grid # Adiciona grade a todos os gráficos

# --- GRÁFICO 1: Latência vs Tamanho da Mensagem (Pequenas: 1-1000 Bytes) ---
set terminal pngcairo size 800,600 enhanced font 'Verdana,10'
set output 'grafico_latencia_pequenas.png'

set title "UDP: Latência vs Tamanho da Mensagem (1-1000 Bytes)"
set xlabel "Tamanho da Mensagem (Bytes)"
set ylabel "Latência Média (ms)" # Convertendo segundos para milissegundos

# Coluna 1: Tamanho da Mensagem (Bytes)
# Coluna 2: Latência (s) -> $2 * 1000 para converter para ms
plot 'data_latency_small.txt' using 1:($2*1000) with linespoints title 'Latência UDP (ms)'


# --- GRÁFICO 2: Vazão vs Tamanho da Mensagem (Pequenas: 1-1000 Bytes) ---
set terminal pngcairo size 800,600 enhanced font 'Verdana,10'
set output 'grafico_vazao_pequenas.png'

set title "UDP: Vazão vs Tamanho da Mensagem (1-1000 Bytes)"
set xlabel "Tamanho da Mensagem (Bytes)"
set ylabel "Vazão Média (Mbps)" # Convertendo bps para Mbps

# Coluna 1: Tamanho da Mensagem (Bytes)
# Coluna 2: Vazão (bps) -> $2 / 1000000.0 para converter para Mbps
plot 'data_throughput_small.txt' using 1:($2/1000000.0) with linespoints title 'Vazão UDP (Mbps)'


# --- GRÁFICO 3: Latência vs Tamanho da Mensagem (Grandes: 1KB-32KB) ---
set terminal pngcairo size 800,600 enhanced font 'Verdana,10'
set output 'grafico_latencia_grandes.png'

set title "UDP: Latência vs Tamanho da Mensagem (1KB-32KB)"
set xlabel "Tamanho da Mensagem (KB)"
set ylabel "Latência Média (ms)" # Convertendo segundos para milissegundos

# Coluna 1: Tamanho da Mensagem (Bytes) -> $1 / 1024.0 para converter para KB
# Coluna 2: Latência (s) -> $2 * 1000 para converter para ms
plot 'data_latency_large.txt' using ($1/1024.0):($2*1000) with linespoints title 'Latência UDP (ms)'


# --- GRÁFICO 4: Vazão vs Tamanho da Mensagem (Grandes: 1KB-32KB) ---
set terminal pngcairo size 800,600 enhanced font 'Verdana,10'
set output 'grafico_vazao_grandes.png'

set title "UDP: Vazão vs Tamanho da Mensagem (1KB-32KB)"
set xlabel "Tamanho da Mensagem (KB)"
set ylabel "Vazão Média (Mbps)" # Convertendo bps para Mbps

# Coluna 1: Tamanho da Mensagem (Bytes) -> $1 / 1024.0 para converter para KB
# Coluna 2: Vazão (bps) -> $2 / 1000000.0 para converter para Mbps
plot 'data_throughput_large.txt' using ($1/1024.0):($2/1000000.0) with linespoints title 'Vazão UDP (Mbps)'

print "Gráficos gerados: grafico_latencia_pequenas.png, grafico_vazao_pequenas.png, grafico_latencia_grandes.png, grafico_vazao_grandes.png"
# Fim do script Gnuplot