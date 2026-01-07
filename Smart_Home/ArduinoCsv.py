import serial
import time
import csv
import threading
from datetime import datetime

# ========================================================
# !!! CONFIGURAÇÃO !!!
# ========================================================
PORTA_R4 = 'COM5'     # Confirma a tua porta
PORTA_NANO = 'COM6'   # Confirma a tua porta
BAUD_RATE = 9600
CSV_FILE = 'dataset_casa_inteligente2.csv'
# ========================================================

# --- MEMÓRIA GLOBAL DO SISTEMA ---
# Guardamos o estado dos sensores e QUEM está em casa
ultima_temp = "0.0"
ultima_hum = "0.0"
usuario_atual = "Ninguem"
uid_atual = "SYSTEM"
fan_state_atual = "0"

# Prepara CSV
try:
    with open(CSV_FILE, 'x', newline='') as f:
        writer = csv.writer(f)
        writer.writerow(["Timestamp", "UID", "User", "Action", "Temp", "Hum", "FanState"])
except FileExistsError:
    pass

def write_csv(data_list):
    try:
        with open(CSV_FILE, 'a', newline='') as f:
            writer = csv.writer(f)
            writer.writerow(data_list)
        print(f"--> [CSV] {data_list}")
    except Exception as e:
        print(f"Erro CSV: {e}")

# --- TAREFA 1: Ouvir R4 (Atualiza Quem está em casa + Sensores) ---
def listen_r4(ser_r4, ser_nano):
    # Importante: Vamos alterar as variáveis globais
    global ultima_temp, ultima_hum, usuario_atual, uid_atual, fan_state_atual
    
    print(f"[R4] A escutar na {PORTA_R4}...")
    while True:
        try:
            if ser_r4.in_waiting:
                line = ser_r4.readline().decode('utf-8', errors='ignore').strip()
                
                if line.startswith("CSV_DATA"):
                    parts = line.split(',')
                    # R4 envia: CSV_DATA, Time, UID, User, Action, Temp, Hum, Fan
                    # Índices: 0=Header, 1=Time, 2=UID, 3=User, 4=Action, 5=Temp, 6=Hum, 7=Fan
                    
                    if len(parts) > 7:
                        # 1. ATUALIZAR A MEMÓRIA GLOBAL (O Segredo!)
                        # Sempre que o R4 fala, atualizamos quem está logado
                        uid_atual = parts[2]
                        usuario_atual = parts[3]
                        
                        ultima_temp = parts[5]
                        ultima_hum = parts[6]
                        fan_state_atual = parts[7]

                        # 2. Criar timestamp do PC
                        timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
                        
                        # 3. Escrever no CSV
                        write_csv([timestamp, uid_atual, usuario_atual, parts[4], ultima_temp, ultima_hum, fan_state_atual])
                    
                elif line:
                    print(f"[R4 Log]: {line}")
        except Exception as e:
            print(f"Erro R4: {e}")
            break

# --- TAREFA 2: Ouvir Nano (Usa a memória do Utilizador) ---
def listen_nano(ser_nano, ser_r4):
    global fan_state_atual
    print(f"[Nano] A escutar na {PORTA_NANO}...")
    while True:
        try:
            if ser_nano.in_waiting:
                line = ser_nano.readline().decode('utf-8', errors='ignore').strip()
                
                timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
                
                # Se detetar voz, usa as variáveis globais 'uid_atual' e 'usuario_atual'
                if line == "EVENT_FAN_ON":
                    print(f">>> [VOZ] LIGAR (Por: {usuario_atual})")
                    ser_r4.write(b"FAN_ON\n")
                    fan_state_atual = "1"
                    
                    # AQUI ESTÁ A CORREÇÃO: Usa o UID e User reais em vez de "Sistema"
                    write_csv([timestamp, uid_atual, usuario_atual, "VOZ_LIGAR", ultima_temp, ultima_hum, "1"])

                elif line == "EVENT_FAN_OFF":
                    print(f">>> [VOZ] DESLIGAR (Por: {usuario_atual})")
                    ser_r4.write(b"FAN_OFF\n")
                    fan_state_atual = "0"
                    
                    # AQUI TAMBÉM:
                    write_csv([timestamp, uid_atual, usuario_atual, "VOZ_DESLIGAR", ultima_temp, ultima_hum, "0"])

        except Exception as e:
            print(f"Erro Nano: {e}")
            break

# --- MAIN ---
if __name__ == "__main__":
    try:
        # Abrir portas
        r4 = serial.Serial(PORTA_R4, BAUD_RATE)
        nano = serial.Serial(PORTA_NANO, BAUD_RATE)
        time.sleep(2)

        # Iniciar Threads
        t1 = threading.Thread(target=listen_r4, args=(r4, nano))
        t2 = threading.Thread(target=listen_nano, args=(nano, r4))
        t1.daemon = True; t2.daemon = True
        t1.start(); t2.start()

        print("\n--- SISTEMA PRONTO (USER UNIFORME) ---")
        while True: time.sleep(1)

    except Exception as e:
        print(f"ERRO FATAL: {e}")