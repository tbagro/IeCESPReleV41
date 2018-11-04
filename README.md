# IeCESPReleV41
Projeto IOT ESP8266 wemos D1 mini, com ferramenta de agendamento

Pojeto de automação  com ferramenta de texto para agendamento de  ações utilizando os comandos abaixo na aba configuração

SCHEDULE SYSTEM FUNCTIONS
* 10/2018 - Andre Michelon
* Options:
*   - Scheduled at a specific Date/Time
*     On (High): SHyyyy-mm-dd hh:mm
*     Off (Low): SLyyyy-mm-dd hh:mm
*   - Monthly
*     On (High): MHdd hh:mm
*     Off (Low): MLdd hh:mm
*   - Weekly
*     On (High): WHd hh:mm 1-7 
*     Off (Low): WLd hh:mm
*   - Daily
*     On (High): DHhh:mm
*     Off (Low): DLhh:mm
*   - Intervaled
*     On (High): IHhh:mm
*     Off (Low): ILhh:mm
* Example Strings:
*   SH2018-10-12 16:30  - set On on Oct 12 2018 16:30
*   MH12 16:30          - set On monthly on day 12 16:30
*   WL6 16:30           - set Off weekly on Fridays 16:30 
*   DH16:30             - set On daily at 16:30
*   IH00:30             - set Off after being On for 30 minutes
*   IL00:10             - set On after being Off for 10 minutes
