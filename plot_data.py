import matplotlib.pyplot as plt
import numpy as np # numpy é útil para lidar com 'NaN' se houver

def load_data(filename):
    """Carrega dados de um arquivo (Tam_Bytes, Valor), tratando 'NaN'."""
    sizes = []
    values = []
    with open(filename, 'r') as f:
        for line in f:
            if line.startswith('#') or not line.strip(): # Ignora comentários e linhas vazias
                continue
            parts = line.split()
            try:
                size = float(parts[0])
                value_str = parts[1].lower() # para tratar 'nan', 'NaN', 'NAN'
                if value_str == 'nan':
                    value = np.nan
                else:
                    value = float(value_str)
                
                sizes.append(size)
                values.append(value)
            except ValueError as e:
                print(f"Aviso: pulando linha mal formatada em {filename}: {line.strip()} - Erro: {e}")
            except IndexError as e:
                print(f"Aviso: pulando linha com colunas insuficientes em {filename}: {line.strip()} - Erro: {e}")

    return np.array(sizes), np.array(values)

def plot_graph(data_file, output_image_file, title, xlabel, ylabel_format_str,
               x_transform_fn=lambda x: x, y_transform_fn=lambda y: y, legend_label_format_str="Valor"):
    """Função genérica para plotar um gráfico."""
    msg_sizes_bytes, raw_values = load_data(data_file)

    # Aplicar transformações
    x_values = x_transform_fn(msg_sizes_bytes)
    y_values = y_transform_fn(raw_values)
    
    # Filtrar NaNs para plotagem (Matplotlib os ignora em plotagens de linha, mas é bom estar ciente)
    valid_indices = ~np.isnan(y_values)
    x_values_plot = x_values[valid_indices]
    y_values_plot = y_values[valid_indices]


    plt.figure(figsize=(10, 6))
    plt.plot(x_values_plot, y_values_plot, marker='o', linestyle='-', label=legend_label_format_str.format("UDP"))
    plt.title(title)
    plt.xlabel(xlabel)
    plt.ylabel(ylabel_format_str)
    plt.grid(True)
    plt.legend(loc='best') # 'best' tenta encontrar a melhor localização automaticamente
    plt.savefig(output_image_file)
    plt.clf() # Limpa a figura para o próximo gráfico
    print(f"Gráfico gerado: {output_image_file}")


if __name__ == "__main__":
    # --- GRÁFICO 1: Latência vs Tamanho da Mensagem (Pequenas: 1-1000 Bytes) ---
    plot_graph(
        data_file='data_latency_small.txt',
        output_image_file='grafico_latencia_pequenas_py.png',
        title='UDP: Latência vs Tamanho da Mensagem (1-1000 Bytes)',
        xlabel='Tamanho da Mensagem (Bytes)',
        ylabel_format_str='Latência Média (ms)',
        y_transform_fn=lambda y: y * 1000, # s para ms
        legend_label_format_str='Latência {} (ms)'
    )

    # --- GRÁFICO 2: Vazão vs Tamanho da Mensagem (Pequenas: 1-1000 Bytes) ---
    plot_graph(
        data_file='data_throughput_small.txt',
        output_image_file='grafico_vazao_pequenas_py.png',
        title='UDP: Vazão vs Tamanho da Mensagem (1-1000 Bytes)',
        xlabel='Tamanho da Mensagem (Bytes)',
        ylabel_format_str='Vazão Média (Mbps)',
        y_transform_fn=lambda y: y / 1000000.0, # bps para Mbps
        legend_label_format_str='Vazão {} (Mbps)'
    )

    # --- GRÁFICO 3: Latência vs Tamanho da Mensagem (Grandes: 1KB-32KB) ---
    plot_graph(
        data_file='data_latency_large.txt',
        output_image_file='grafico_latencia_grandes_py.png',
        title='UDP: Latência vs Tamanho da Mensagem (1KB-32KB)',
        xlabel='Tamanho da Mensagem (KB)',
        ylabel_format_str='Latência Média (ms)',
        x_transform_fn=lambda x: x / 1024.0, # Bytes para KB
        y_transform_fn=lambda y: y * 1000,   # s para ms
        legend_label_format_str='Latência {} (ms)'
    )

    # --- GRÁFICO 4: Vazão vs Tamanho da Mensagem (Grandes: 1KB-32KB) ---
    plot_graph(
        data_file='data_throughput_large.txt',
        output_image_file='grafico_vazao_grandes_py.png',
        title='UDP: Vazão vs Tamanho da Mensagem (1KB-32KB)',
        xlabel='Tamanho da Mensagem (KB)',
        ylabel_format_str='Vazão Média (Mbps)',
        x_transform_fn=lambda x: x / 1024.0, # Bytes para KB
        y_transform_fn=lambda y: y / 1000000.0, # bps para Mbps
        legend_label_format_str='Vazão {} (Mbps)'
    )