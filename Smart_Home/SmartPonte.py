import serial
import time
import threading

# ==========================================
# CONFIRMA AS TUAS PORTAS!
PORTA_R4 = 'COM5'     
PORTA_NANO = 'COM6'   # A porta do Nano Sense (que muda no reset)
BAUD_RATE = 9600
# ==========================================

def listen_r4(ser_r4, ser_nano):
    print(f"--- A escutar R4 na {PORTA_R4}... ---")
    while True:
        try:
            if ser_r4.in_waiting:
                line = ser_r4.readline().decode('utf-8', errors='ignore').strip()
                
                if line.startswith("CSV_DATA"):
                    parts = line.split(',')
                    if len(parts) > 6:
                        nome_user = parts[3].strip() 
                        try:
                            # Ler valores crus
                            temp_real = float(parts[5].strip())
                            hum_real = float(parts[6].strip())
                        except:
                            continue

                        # === LÓGICA IGUAL AO TEU TREINO ===
                        
                        # 1. USER: Tu definiste -100 para Mãe e 100 para Pai no treino
                        user_val = 0
                        if 'Mae' in nome_user or 'Mãe' in nome_user:
                            user_val = -100
                        elif 'Pai' in nome_user:
                            user_val = 100
                        else:
                            user_val = 0 # Filho/Outros

                        # 2. TEMP e HUM: ENVIAR PURO (Sem subtrair nada!)
                        # O cast para int é só para tirar casas decimais, o valor mantem-se (ex: 18)
                        val_temp = int(temp_real)
                        val_hum = int(hum_real)

                        # ENVIAR PARA O NANO
                        # Exemplo: DATA:18,70,-100
                        msg = f"DATA:{val_temp},{val_hum},{user_val}\n"
                        ser_nano.write(msg.encode('utf-8'))
                        
                        print(f"R4: {nome_user} ({temp_real}C) -> A Enviar: {msg.strip()}")

        except Exception as e:
            print(f"Erro R4: {e}")
            break

def listen_nano(ser_nano):
    print(f"--- A escutar Nano na {PORTA_NANO}... ---")
    while True:
        try:
            if ser_nano.in_waiting:
                # Ver o que o Nano diz (Diagnóstico)
                line = ser_nano.readline().decode('utf-8', errors='ignore').strip()
                print(f"[NANO]: {line}")
        except:
            break

if __name__ == "__main__":
    try:
        r4 = serial.Serial(PORTA_R4, BAUD_RATE)
        nano = serial.Serial(PORTA_NANO, BAUD_RATE)
        time.sleep(2)
        
        t1 = threading.Thread(target=listen_r4, args=(r4, nano))
        t2 = threading.Thread(target=listen_nano, args=(nano,))
        t1.start(); t2.start()

    except Exception as e:
        print(f"Erro Portas: {e}")