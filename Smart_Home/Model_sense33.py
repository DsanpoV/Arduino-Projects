import pandas as pd
import numpy as np
import tensorflow as tf
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import LabelEncoder

CSV_FILE = 'dataset_casa_inteligente2.csv'
MODEL_FILENAME = 'fan_model'

# 1. Carregar
try:
    df = pd.read_csv(CSV_FILE)
except:
    print("ERRO: CSV não encontrado.")
    exit()

# Limpeza Básica
df = df[pd.to_numeric(df['Temp'], errors='coerce').notnull()]
df['Temp'] = df['Temp'].astype(float)
df['Hum'] = df['Hum'].astype(float)
df['FanState'] = df['FanState'].astype(int)

# 2. LABEL ENCODING COM MAXIMIZAÇÃO DE RANGE (A TUA IDEIA)
# Primeiro vemos quem é quem
le = LabelEncoder()
df['User_Code_Original'] = le.fit_transform(df['User'].astype(str))
mapping = dict(zip(le.classes_, le.transform(le.classes_)))
print("Mapping Original:", mapping)

# AGORA A MUDANÇA TÉCNICA:
# Vamos afastar matematicamente as classes para os extremos do INT8.
# Assumindo: Filho/Ninguem(0), Mae(1), Pai(2) - Ajusta conforme o teu mapping acima!
# Vamos transformar em: Mãe = -100, Filho = 0, Pai = 100
# (Não usamos 127 para dar margem de segurança contra overflow)

def maximizar_range(val):
    # ATENÇÃO: Confirma se estes números batem certo com o teu 'Mapping Original' impresso acima
    if val == 1: return -100.0 # Mãe (Desligar) -> Extremo Negativo
    if val == 2: return 100.0  # Pai (Ligar)    -> Extremo Positivo
    return 0.0                 # Filho/Outros   -> Centro

df['User_Code_Max'] = df['User_Code_Original'].apply(maximizar_range)

print("Novo User Input (Amostra):")
print(df[['User', 'User_Code_Max']].head())

# 3. Preparar Dados (Usamos o User_Code_Max agora)
X = df[['Temp', 'Hum', 'User_Code_Max']].values.astype('float32')
y = df['FanState'].values.astype('float32')

X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42)

# 4. Modelo (Simples mas eficaz)
model = tf.keras.Sequential([
    tf.keras.layers.Dense(16, activation='relu', input_shape=(3,)),
    tf.keras.layers.Dense(16, activation='relu'),
    tf.keras.layers.Dense(1, activation='sigmoid')
])

model.compile(optimizer='adam', loss='binary_crossentropy', metrics=['accuracy'])
model.fit(X_train, y_train, epochs=200, batch_size=8, verbose=0)

# 5. Quantização FULL INT8 (Com representação de dataset correta)
converter = tf.lite.TFLiteConverter.from_keras_model(model)
converter.optimizations = [tf.lite.Optimize.DEFAULT]

def representative_data_gen():
    for input_value in X_train:
        # Importante: O dataset representativo agora tem os valores -100 e 100!
        # O conversor vai aprender que a escala tem de abranger isto tudo.
        yield [np.array([input_value], dtype=np.float32)]

converter.representative_dataset = representative_data_gen
converter.target_spec.supported_ops = [tf.lite.OpsSet.TFLITE_BUILTINS_INT8]
converter.inference_input_type = tf.int8
converter.inference_output_type = tf.int8

tflite_model = converter.convert()

# 6. Gravar .H
hex_array = [format(val, '#04x') for val in tflite_model]
c_array = ", ".join(hex_array)
header = f"const unsigned char g_fan_model[] = {{ {c_array} }};\nconst int g_fan_model_len = {len(tflite_model)};"

with open(f'{MODEL_FILENAME}.h', 'w') as f:
    f.write(header)

print("SUCESSO. O modelo agora espera entradas -100 (Mãe) e 100 (Pai).")