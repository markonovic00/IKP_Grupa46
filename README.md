# IKP_Grupa46

Implementirati sistem za dostavljanje hrane. Sistem se sastoji iz 3 komponente: servis, dostavljač i
klijent.
Klijenti šalju servisu poruku gde navode šta naručuju, adresu i hitnost.
Servis kada dobije poruku od klijenta, proveri stanje, izračuna cenu i proveri da li može da ispuni zadatu
hitnost (na osnovu udaljenosti klijenata i dostupnosti dostavljača). Nakon toga angažuje dostavljače.
Servis treba da optimizuje slanje grupisanjem paketa tako da jedan dostavljač u jednom prolazu može da
dostavi više paketa.
Dostavljači čekaju da ih servis angažuje. Njihov zadatak je da dostave paket klijentu i da sačekaju
klijentov potpis.
Na početku rada postoji fiksan broj dostavljača. Servis može da poveća ovaj broj ukoliko dođe do
preopterećenja.
