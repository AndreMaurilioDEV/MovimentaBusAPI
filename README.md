# 🚌 MovimentaBus — Sistema de Contagem Inteligente de Passageiros

### O MovimentaBus é um sistema IoT desenvolvido para monitoramento em tempo real do fluxo de passageiros dentro de ônibus através de sensores ultrassônicos e envio automático de dados para uma API web. O objetivo é fornecer informações relevantes para gestão de transporte público, como lotação, entradas, saídas, ocupação atual e status operacional.

--- 

## 📌 Funcionalidades

- Contagem automática de passageiros com 4 sensores ultrassônicos
- Detecção inteligente de fluxo (entrada e saída)
- Sistema antifraude com confirmação de sensores
-  Envio automático dos dados para API com debounce inteligente
- Dashboard web exibindo informações em tempo real
- Suporte Wi-Fi com reconexão automática
- Timestamp gerado corretamente no backend
- Estrutura preparada para expansão (ex.: GPS, IA, Edge Computing)

---

## 🧠 Arquitetura do Sistema

```plaintext

[Sensores Ultrassônicos] 
          ↓
[ESP32 com algoritmo de validação]
          ↓
      (Wi-Fi Secure)
          ↓
   [API Node.js hospedada]
          ↓
   [Dashboard Web em Tempo Real]



```

---

## 💻 Software

```plaintext
| Parte | Tecnologia |
|--------|------------|
| Firmware | Arduino IDE + C/C++ |
| Backend | Node.js + Express |
| Banco de Dados | (opcional — configurável) |
| Frontend | HTML, CSS, JavaScript |
| Comunicação | HTTPS com `WiFiClientSecure` |

```

---

## 🚦 Lógica de Contagem

- Entrada é confirmada quando:

```plaintext

Sensor Externo → Sensor Interno

```

- Saída é confirmada quando:

```plaintext

Sensor Interno → Sensor Externo

```

#### O sistema utiliza timeout e lógica de confirmação para evitar falsos positivos causados por obstáculos temporários.

---

## 📊 Dashboard
O dashboard exibe, em tempo real:

- Número total de passageiros
- Últimas entradas e saídas
- Status de conexão
- Identificação do ônibus e linha
- Timestamp formatado com "pt-BR" e zona "America/Sao_Paulo"

---

#### Projeto acadêmico e experimental focado em IoT inteligente para transporte público.
