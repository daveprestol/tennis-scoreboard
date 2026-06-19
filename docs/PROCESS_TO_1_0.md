# Tennis Scoreboard OBS Plugin - Proceso hasta la version 1.0

Este documento resume el camino tecnico seguido para convertir la idea inicial en una version 1.0 funcional del plugin `tennis-scoreboard` para OBS Studio en macOS.

## Objetivo inicial

Crear un plugin nativo para OBS en macOS que permitiera manejar una pizarra de tenis en vivo:

- Control del partido desde OBS.
- Overlay visual para colocarlo como Browser Source.
- Soporte para singles y doubles.
- Score automatico de tenis, con soporte para uso tipo padel mediante punto de oro.
- Avisos basicos como `Break point`, `Set point`, `Match point`, `Game point`, `Deuce`, `Golden Point`, `Advantage / Ad`, `Tiebreak`, `Serving for the set` y `Serving for the match`.
- Configuracion visual del board, nombres, colores, logos y datos del evento.

## Herramientas usadas

### Instaladas/configuradas localmente

- Xcode 26.5, build `17F42`.
- Homebrew 6.0.2.
- CMake 4.3.3, instalado/confirmado via Homebrew.
- Git 2.53.0, instalado/confirmado via Homebrew.
- Ninja 1.13.2, instalado/confirmado via Homebrew.

El script local de instalacion usa Xcode como generador de CMake para macOS. El preset actual esta orientado a Apple Silicon:

```sh
cmake --preset macos
cmake --build --preset macos
```

### Dependencias descargadas por el template de OBS

El proyecto esta basado en el OBS Plugin Template. Las dependencias se declaran en `buildspec.json`:

- OBS Studio sources `31.1.1`.
- Pre-built `obs-deps` version `2025-07-11`.
- Pre-built Qt6 version `2025-07-11`.

Estas dependencias se descargan y preparan durante el build de CMake.

## Estructura tecnica creada

El plugin quedo organizado en estas piezas principales:

- `src/plugin-main.cpp`: entrada del modulo OBS y registro del dock `Docks > Tennis Scoreboard`.
- `src/ui/ScoreboardDock.*`: dock pequeno dentro de OBS. Inicia/detiene el servidor local y muestra URLs de uso.
- `src/overlay/OverlayHttpServer.*`: servidor HTTP interno con dos puertos:
  - `http://127.0.0.1:9876/config`
  - `http://127.0.0.1:19876/score`
- `src/core/MatchState.*`: estado central del partido y serializacion JSON.
- `src/core/TennisRules.*`: reglas de puntuacion, sets, games, tiebreak, undo y bloqueo al terminar el partido.
- `resources/overlay/*`: HTML/CSS/JS del panel de configuracion y del scoreboard visual.
- `resources/default-presets/default-dark.json`: preset visual por defecto.
- `tests/tennis-rules-tests.cpp`: pruebas rapidas del motor de reglas.

## Evolucion funcional

### 1. Plugin nativo y dock inicial

Primero se creo el plugin nativo usando C++ y Qt. La idea inicial era que OBS abriera un controlador dentro de un dock.

Luego se redujo el alcance del dock: en vez de tener todo el panel gigante dentro de OBS, el dock quedo como un panel minimo que:

- Arranca el servidor cuando se abre.
- Muestra si el servidor esta corriendo.
- Muestra URLs para configurar y para crear el Browser Source.
- Permite detener/iniciar el servidor.
- Avisa que el dock puede cerrarse y el servidor sigue corriendo.

### 2. Servidor local para config y score

Se separaron las interfaces:

- `/config` en puerto `9876`: panel para operar y configurar el partido desde el Mac o desde otro dispositivo en la misma red.
- `/score` en puerto `19876`: overlay visual para agregar en OBS como Browser Source.

El servidor expone tambien:

- `/state.json`: estado actual del partido en JSON.
- `/api/action`: endpoint para acciones del panel de configuracion.

### 3. Configuracion remota

El panel `/config` permite:

- Nombre del evento.
- Logo del evento por URL.
- Tint opcional del logo del evento.
- Singles/doubles.
- Nombres de jugadores/equipos.
- Formato del partido:
  - Mejor de X sets.
  - Games por set.
  - Win by games.
  - Tiebreak at.
  - Modo de game: ventaja tradicional o punto de oro.
- Colores:
  - Board.
  - Row.
  - Accent.
  - Event title.
  - Text.
  - Team A.
  - Team B.
- Opacidad global del scoreboard.
- Acciones de score:
  - Point A/B.
  - Serve A/B.
  - Start.
  - Undo.
  - Reset game.
  - Reset set.
  - Reset match.
  - Finish.
  - Suspended.
  - Interrupted.
  - Delayed.
  - RET.
  - WO.
  - Custom notice.

### 4. Scoreboard visual

El overlay `/score` quedo como una pizarra compacta para OBS:

- Nombre del evento.
- Logo opcional.
- Sets ganados.
- Games del set actual.
- Puntos del game actual.
- Indicador de quien sirve.
- Avisos automaticos de tenis.
- Pantalla de ganador al terminar el partido.
- Pantallas de estado para delayed/suspended/interrupted/RET/WO/custom.

El placeholder final del evento es `Your Event Name`, y los jugadores por defecto son `Player 1` y `Player 2`.

## Problemas tecnicos que aparecieron y como se resolvieron

### CMake no encontraba compilador C/C++

Error:

```text
The C compiler identification is unknown
The CXX compiler identification is unknown
No CMAKE_C_COMPILER could be found.
No CMAKE_CXX_COMPILER could be found.
```

Causa probable: Xcode o sus herramientas no estaban correctamente instaladas/seleccionadas.

Solucion aplicada/documentada:

```sh
sudo xcode-select -s /Applications/Xcode.app/Contents/Developer
sudo xcodebuild -license accept
```

Despues se pudo correr el script de instalacion local.

### OBS mostraba que el plugin fallo al cargar

Mensaje:

```text
The following OBS plugins failed to load:
tennis-scoreboard
```

Se trabajo sobre el build/install local para que el bundle terminara en:

```text
~/Library/Application Support/obs-studio/plugins/tennis-scoreboard.plugin
```

El script final:

- Compila el plugin.
- Copia el `.plugin` al folder de OBS.
- Limpia atributos extendidos con `xattr -cr`.
- Firma localmente con `codesign --force --deep --sign -`.

Verificacion final:

```sh
codesign --verify --deep --strict --verbose=2 "$HOME/Library/Application Support/obs-studio/plugins/tennis-scoreboard.plugin"
```

Resultado:

```text
valid on disk
satisfies its Designated Requirement
```

### OBS cerraba con "OBS quitted unexpectedly"

Al cerrar OBS aparecia un crash. Se ajusto el manejo del dock en `plugin-main.cpp` para evitar destruirlo en el unload de una forma que chocara con el ciclo de vida de OBS/Qt.

El plugin mantiene el dock registrado sin forzar una destruccion manual conflictiva al salir de OBS.

### `/score not found`

El servidor inicialmente no estaba sirviendo correctamente la ruta del overlay o habia confusion entre puertos.

Se corrigio el enrutamiento:

- `/config` vive en el servidor de config, puerto `9876`.
- `/score` vive en el servidor de score, puerto `19876`.
- Los assets compartidos como CSS/JS y `state.json` se sirven desde ambos segun la ruta.

### El config no guardaba nombres ni evento

El panel hacia polling a `/state.json` cada segundo y podia pisar lo que el usuario estaba escribiendo.

Solucion:

- Se agrego manejo de `setupDirty`.
- Se evita actualizar un input activo mientras el usuario escribe.
- Se usa autosave con debounce.
- Se fuerza refresh del setup solo cuando corresponde, por ejemplo despues de guardar.

### Espacios y caracteres raros en nombres

Hubo problemas con nombres que no aceptaban espacios o se transformaban al perder foco.

Solucion:

- El panel envia JSON a `/api/action`.
- El servidor parsea JSON con Qt (`QJsonDocument` / `QJsonObject`), no una manipulacion fragile de query strings.
- Los nombres se guardan como texto completo.

### Doubles mostraba solo apellidos

En un momento, doubles formateaba `Andy Perez / Manny Cruz` como `Perez/Cruz`. Para el uso real del usuario se cambio a conservar el nombre completo.

Ahora doubles permite:

- Campo separado por jugador.
- O un solo campo con formato `Andy Perez/Manny Cruz`.

El display final queda `Andy Perez/Manny Cruz`.

### Layout del scoreboard se rompia con nombres largos

Primero el ancho dinamico de nombres movia las columnas de sets/games/points. Luego los encabezados dejaron de alinearse con las cifras.

Solucion:

- Se cambio el layout de score a una tabla real en el HTML.
- Las columnas de `SETS`, `GAMES` y `PTS` tienen anchos fijos.
- La columna de nombres calcula su ancho por el nombre mas largo de ambos equipos, dejando un margen visual.

Asi los nombres pueden crecer sin mover las cifras ni los headers.

### Avisos de score mostraban demasiadas cosas

Al principio el sistema podia mostrar varios terminos al mismo tiempo: por ejemplo `Break point` y `Game point`.

Se simplifico para transmision:

Prioridad actual:

1. `Match point`
2. `Set point`
3. `Deuce`
4. `Break point`
5. `Game point`
6. `Advantage / Ad`
7. `Tiebreak`
8. `Serving for the match`
9. `Serving for the set`

Ademas, `Break point` reemplaza a `Game point` cuando aplica.

Se corrigio tambien que 40-40 no se marcara como game/break point, sino como `Deuce`.

### Fin de partido

Cuando alguien gana el match:

- Se bloquea seguir sumando puntos.
- El score deja de mostrar la tabla normal.
- Muestra solamente el nombre del ganador y `WINNER` o `WINNERS`.
- El usuario debe usar `Reset Match` para empezar de nuevo.

### Estados especiales

Los botones `Suspended`, `Interrupted`, `Delayed`, `RET` y `WO` quedaron funcionales. Al presionarlos, el overlay muestra un cartel grande en pantalla.

Tambien se agrego un boton `Custom`, donde el usuario escribe cualquier mensaje y lo muestra como aviso.

### Logo y tint

Se agrego:

- URL de logo de evento.
- Tint opcional para logo.

Cuando el tint esta activado, el overlay intenta usar el logo como mascara CSS y aplicar el color elegido. Esto funciona mejor con logos solidos PNG/SVG. Si el browser no puede aplicar mask por restricciones del asset, el logo normal sigue siendo la opcion segura.

### Opacidad del board

Se agrego un slider `Scoreboard opacity` en `/config`. Controla la opacidad global del scoreboard completo.

### Ajustes finales del UI de configuracion

Se corrigieron problemas visuales del panel `/config`:

- Los inputs y sliders ahora usan `box-sizing: border-box` para no salirse del panel.
- `Tiebreak at` y `Scoreboard opacity` mantienen margen derecho correcto.
- Los campos de jugadores se reorganizaron en una grilla de dos columnas iguales.
- `Player 1` y `Player 2` quedan con el mismo ancho visual.

### Modo punto de oro para padel/no-ad

Se agrego un selector `Game scoring` con dos opciones:

- `Traditional advantage`: al llegar a 40-40 se juega ventaja y luego game.
- `Golden point`: al llegar a 40-40 el overlay muestra `Golden Point`; el siguiente punto gana el game.

El modo se guarda en `MatchFormat` como `gameScoringMode`, viaja en el JSON de estado y es usado por `TennisRules` al calcular el game.

## Comandos importantes

### Build local

```sh
cmake --preset macos
cmake --build --preset macos
```

### Instalar en OBS localmente

```sh
./scripts/install-local-macos.sh
```

### Probar reglas

```sh
clang++ -std=c++17 -Isrc tests/tennis-rules-tests.cpp src/core/TennisRules.cpp src/core/MatchState.cpp -o /tmp/tennis-rules-tests
/tmp/tennis-rules-tests
```

Resultado final esperado:

```text
tennis-rules-tests passed
```

### Verificar firma del plugin instalado

```sh
codesign --verify --deep --strict --verbose=2 "$HOME/Library/Application Support/obs-studio/plugins/tennis-scoreboard.plugin"
```

## Estado final de la version 1.0

La version 1.0 queda lista para uso local en OBS Studio en macOS Apple Silicon:

- Plugin nativo carga en OBS.
- Dock aparece en `Docks > Tennis Scoreboard`.
- Dock inicia el servidor local.
- Config esta disponible en `http://127.0.0.1:9876/config`.
- Score esta disponible en `http://127.0.0.1:19876/score`.
- Browser Source de OBS debe apuntar a `/score`.
- Operador puede manejar el partido desde `/config`.
- Otro dispositivo en la misma red puede abrir `/config` usando la IP local del Mac.
- El usuario final puede instalar un ZIP compilado sin Xcode, copiando `tennis-scoreboard.plugin` al folder de plugins de OBS y ejecutando `xattr`/`codesign` si macOS lo bloquea.

## Limitaciones conocidas para futuras versiones

- El plugin todavia no tiene instalador firmado/notarizado para distribuir publicamente en macOS.
- No hay persistencia formal en disco para sesiones/presets entre reinicios.
- El tint de logo depende del comportamiento CSS mask del browser y del tipo/origen del asset.
- El preset de CMake actual esta orientado a macOS Apple Silicon (`arm64`).
- Windows/Linux existen en el template, pero la version trabajada y probada aqui fue macOS.
