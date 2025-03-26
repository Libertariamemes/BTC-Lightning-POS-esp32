Máquina POS Bitcoin com ESP32 e Lightning Network
🚀 Pagamentos Bitcoin rápidos, de baixo custo e descentralizados

Visão Geral
Este projeto é uma máquina de ponto de venda (POS) Bitcoin de código aberto, alimentada por um microcontrolador ESP32, utilizando a Lightning Network para transações instantâneas e de baixo custo. Ele permite que os comerciantes aceitem pagamentos em Bitcoin de forma simples, sem depender de instituições financeiras tradicionais.

Funcionalidades
✅ Transações Rápidas – Os pagamentos são confirmados instantaneamente via Lightning Network.
✅ Baixo Custo e Escalável – Taxas quase nulas tornam ideal para pequenos negócios.
✅ Conectividade WiFi – Conecta-se à internet via WiFi para transações em tempo real.
✅ Suporte a QR Code – Gera faturas Lightning como QR codes para pagamento fácil.
✅ Tela Sensível ao Toque Capacitiva – Interface de usuário suave e responsiva.
✅ UI LVGL – Interface de usuário alimentada por LVGL, com telas geradas por código com o EEZ Studio by Envox.
✅ Autônoma e Segura – Atualmente usa o Alby de terceiros, mas pode ser trocado para LDN conforme o usuário preferir.
✅ Código Aberto – Totalmente personalizável e de código aberto, movido pela comunidade.

Como Funciona
O comerciante insere o valor a ser pago no dispositivo baseado em ESP32.

O dispositivo se comunica com Alby e LNbits para gerar uma fatura Lightning.

A fatura é exibida como um QR code na tela do dispositivo.

O cliente escaneia o QR code com uma carteira Lightning e conclui o pagamento.

O pagamento é verificado instantaneamente, e uma mensagem de confirmação é exibida.

Requisitos de Hardware
Microcontrolador ESP32

Tela TFT com Tela Sensível ao Toque Capacitiva

Conectividade WiFi

Fonte de alimentação (USB ou bateria)

Pilha de Software
LVGL (Light and Versatile Graphics Library) – Usado para criar a interface de usuário na tela TFT.

Firmware ESP32 (escrito em C++ usando o framework Arduino)

Alby (de terceiros, atualmente usado) e LNbits como backend Lightning (pode ser trocado para LDN conforme o usuário preferir)

API REST / WebSockets para faturas Lightning

Biblioteca de geração de QR Code

EEZ Studio by Envox – Usado para gerar o código das telas de interface para LVGL.

Instruções de Configuração
Grave o firmware no ESP32 com o firmware fornecido.

O POS irá ativar seu Ponto de Acesso (AP). Conecte seu celular à rede do ESP32 e use a interface web para configurar as credenciais WiFi e as credenciais LNbits.

Alby é sempre usado através do portal LNbits para pagamentos Lightning.

Implemente e comece a aceitar pagamentos Lightning instantaneamente!

Contribuindo
Contribuições são bem-vindas! Fique à vontade para enviar problemas, pull requests ou discutir melhorias no repositório.

Licença
Este projeto é de código aberto sob a Licença MIT.

💡 Junte-se à revolução Bitcoin e habilite pagamentos instantâneos e sem fronteiras com ESP32 e Lightning! ⚡

