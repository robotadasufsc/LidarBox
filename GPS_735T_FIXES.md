# Problemas e Soluções do GPS GP-735T

## Problemas Identificados no Código Original

### 1. **Comandos UBX com Checksums Incorretos**
- Os comandos binários UBX não tinham checksums válidos
- Resultado: GPS ignorava os comandos de configuração

### 2. **Timing Inadequado**
- Delays muito curtos (150ms) entre comandos
- Não aguardava ACK/NACK do GPS
- Resultado: Comandos perdidos ou corrompidos

### 3. **Configuração Inconsistente**
- Parâmetros de configuração NMEA incorretos
- Comando de salvamento com tamanho errado
- Resultado: Configurações não eram salvas

### 4. **Falta de Tratamento de Respostas**
- Código original descartava respostas do GPS
- Não verificava se comandos foram aceitos
- Resultado: Impossível diagnosticar problemas

## Soluções Implementadas

### Versão 1: UBX Corrigido (`adhtech-gt-735t.cc`)
- ✅ Checksums UBX corrigidos
- ✅ Delays aumentados para 250ms entre comandos
- ✅ Limpeza adequada do buffer entre comandos
- ✅ Comando de salvamento corrigido
- ✅ Debug melhorado

### Versão 2: NMEA Alternativo (`adhtech-gt-735t-nmea.cc`)
- ✅ Comandos NMEA/SiRF para compatibilidade
- ✅ Sintaxe similar ao módulo EM506 funcional
- ✅ Fallback caso UBX não funcione

## Como Usar

### Para Testar a Versão UBX Corrigida (Recomendado):
Já está ativo no `platformio.ini` com `GPS_ADHTECH_GT_735T=1`

### Para Testar a Versão NMEA Alternativa:
Edite o `platformio.ini`:
```ini
build_flags = -DLIDAR_BENEWAKE_TF02=1 -DGPS_ADHTECH_GT_735T_NMEA=1 -DDEBUG_TO_SERIAL -DDEBUG_NMEA
```

## Diagnóstico

Para verificar se o GPS está funcionando:

1. **Ative DEBUG_NMEA** no platformio.ini
2. **Monitore a saída serial** durante a inicialização
3. **Procure por**:
   - Mensagens NMEA ($GPGGA, $GPRMC)
   - Dados de localização válidos
   - Número de satélites > 0

## Comandos UBX Explicados

| Comando | Função | Checksum |
|---------|--------|----------|
| `F0 01 00` | Desabilita GLL | Corrigido |
| `F0 02 00` | Desabilita GSA | Corrigido |
| `F0 03 00` | Desabilita GSV | Corrigido |
| `F0 05 00` | Desabilita VTG | Corrigido |
| `F0 08 00` | Desabilita ZDA | Corrigido |
| `F0 00 01` | Habilita GGA | Corrigido |
| `F0 04 01` | Habilita RMC | Corrigido |

## Próximos Passos

Se ainda houver problemas:

1. **Verificar conexões físicas** (TX/RX, VCC, GND)
2. **Testar com diferentes baud rates** (4800, 9600, 38400)
3. **Verificar se o módulo está funcionando** com software de teste
4. **Considerar reset de fábrica** do módulo GPS

## Diferenças entre Módulos

| Característica | EM506 | GP-735T |
|----------------|-------|---------|
| Protocolo | NMEA/SiRF | UBX/NMEA |
| Baud Rate | 4800 | 9600 |
| Comandos | Texto | Binário/Texto |
| Complexidade | Simples | Moderada |
