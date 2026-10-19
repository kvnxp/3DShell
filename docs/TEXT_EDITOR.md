# Editor de Texto - 3DShell

## Descripción
El editor de texto integrado permite abrir y editar archivos de texto plano directamente desde 3DShell. Está diseñado como un editor simple similar a Notepad, optimizado para la interfaz de Nintendo 3DS.

Se puede abrir de dos formas:
- Desde el **app drawer**: la app **Notepad** abre un documento nuevo en blanco.
- Desde **File Explorer**: al seleccionar un archivo de texto.

## Archivos Soportados
Al seleccionar en File Explorer un archivo con alguna de estas extensiones, se abre en el editor:
- `.txt` - Archivos de texto plano
- `.log` - Archivos de registro
- `.json` - Archivos JSON
- `.xml` - Archivos XML
- `.ini` - Archivos de configuración INI
- `.cfg` - Archivos de configuración
- `.md` - Archivos Markdown
- `.cpp`, `.c` - Código fuente C/C++
- `.h`, `.hpp` - Headers C/C++
- `.py` - Scripts Python
- `.js` - Scripts JavaScript
- `.html` - Páginas HTML
- `.css` - Hojas de estilo CSS
- `.csv` - Tablas CSV
- `.yml`, `.yaml` - Archivos YAML
- `.toml` - Archivos de configuración TOML
- `.lua` - Scripts Lua
- `.sh` - Scripts de shell
- `.rs` - Código fuente Rust
- `.s`, `.asm` - Código ensamblador

## Interfaz del Editor

### Modo Vista (View)
- **Pantalla superior**: Muestra el contenido del archivo
- **Pantalla inferior**: Título "Text Reader" y botones táctiles Edit, Save y Back. Muestra "* unsaved" si hay cambios sin guardar
- **Barra de título**: Nombre del archivo (con "*" si hay cambios sin guardar)
- **Barra de estado**: Posición del cursor (línea y columna) y modo actual
- **Controles**: A: editar, X: guardar, B: cerrar el documento (ver [Limitaciones](#limitaciones)), D-pad: mover el cursor

### Modo Edición (Edit)
- **Pantalla superior**: Editor de texto con cursor visible
- **Pantalla inferior**: Teclado virtual personalizado con múltiples layouts
- **Cursor visible**: Parpadea en la posición actual
- **Navegación**: D-pad para mover el cursor
- **Controles:** A o X: guardar, B: volver al modo vista (conserva los cambios), START: guardar y volver al modo vista, SELECT: editar la línea actual con el teclado del sistema (ver [Pendiente de revisión](#pendiente-de-revisión))
- **Teclado virtual**: Se usa con el touch en la pantalla inferior; ver la sección de teclado más abajo

## Funcionalidades

### Navegación
- **Flechas direccionales**: Mover cursor por línea/columna
- **Scroll automático**: La vista se desplaza cuando el cursor se acerca al borde

### Edición
- **Teclado virtual personalizado**: 
  - Layout QWERTY con letras minúsculas
  - Layout con letras mayúsculas (presionar SHIFT)
  - Layout numérico (presionar 123)
  - Layout de símbolos (presionar #+=)
  - Fila inferior: SHIFT, 123, espacio, BKSP y ENTER
- **Inserción**: El texto se inserta en la posición del cursor
- **Borrado**: Retroceso elimina caracteres hacia atrás
- **Nueva línea**: Enter crea una nueva línea

### Archivo
- **Carga automática**: Al abrir un archivo, se carga su contenido
- **Guardado manual**: Presionar X en modo vista o A en modo edición
- **Documento nuevo**: Al abrir Notepad desde el app drawer se crea un documento en blanco. Al guardar, se pide un nombre; si no tiene extensión se agrega `.txt`, y se guarda en el directorio actual. No sobrescribe archivos existentes
- **Abrir otro archivo**: El botón Open de la barra táctil guarda los cambios pendientes (si el guardado falla, no se sale) y abre File Explorer para elegir un archivo
- **Indicador de cambios**: "*" en el título cuando hay modificaciones sin guardar

## Teclado Virtual

### Layouts Disponibles
1. **QWERTY Minúsculas**: Letras minúsculas estándar
2. **QWERTY Mayúsculas**: Letras mayúsculas (accesible con ⇧)
3. **Numérico/Símbolos**: Números y símbolos comunes (accesible con 123)
4. **Símbolos Extendidos**: Símbolos adicionales (accesible con #+=)

### Controles del Teclado
- **Táctil**: Tocar las teclas para insertar caracteres
- **BKSP**: Elimina el carácter anterior
- **Espacio**: Inserta un espacio
- **ENTER**: Inserta una nueva línea
- **SHIFT**: Cambia a mayúsculas (y vuelve a minúsculas)
- **123**: Cambia al layout numérico (y vuelve a letras con **ABC**)
- **#+=**: Cambia al layout de símbolos

### Características
- Diseño optimizado para pantalla táctil de 3DS
- Feedback visual al presionar teclas
- Inserción directa en la posición del cursor del editor
- Al entrar al modo edición, el teclado vuelve al layout de minúsculas

## Limitaciones
- **Líneas máximas**: 1000 líneas por archivo
- **Tamaño máximo**: Al abrir, solo se leen los primeros 256 KB del archivo
- **Longitud de línea**: Limitada a 50 caracteres visibles
- **Codificación**: Solo UTF-8 básico
- **Funciones avanzadas**: Sin búsqueda/reemplazo, deshacer/rehacer
- **Cierre sin confirmación**: HOME, X de la barra táctil y B en modo vista cierran el documento sin preguntar por los cambios sin guardar
- **Reemplazo al abrir**: Abrir un archivo desde File Explorer reemplaza el documento actual sin guardar

## Arquitectura del Código

### Archivos
- `source/shell/notepad/notepad.h` / `notepad.cpp` - Editor: carga, guardado, edición y render. `IsTextFile()` define las extensiones
- `source/shell/gui/screens/notepad_screen.h` / `notepad_screen.cpp` - App de pantalla completa del app drawer (barra táctil, Open, HOME, X)
- `include/apps.h` - Registro de aplicaciones del app drawer (incluye Notepad)
- `source/shell/osk/virtual_keyboard.h` / `virtual_keyboard.cpp` - Teclado virtual de la pantalla inferior
- `source/shell/gui/screens/file_explorer_screen.cpp` - Abre archivos de texto con `Notepad::IsTextFile()`
- `source/ui/screen_manager.cpp` - Atajo global de SELECT (Task Manager)

### Modos
- `NOTEPAD_VIEW` - Modo de solo lectura (navegación)
- `NOTEPAD_EDIT` - Modo de edición

### Estructura de Datos
```cpp
struct NotepadData {
    std::string filename;            // Nombre del archivo
    std::string filepath;            // Ruta completa (vacía si el documento no tiene nombre)
    std::vector<std::string> lines;  // Contenido por líneas
    int cursor_line;                 // Línea actual del cursor
    int cursor_col;                  // Columna actual del cursor
    int scroll_offset;               // Desplazamiento vertical
    int scroll_x;                    // Desplazamiento horizontal
    NotepadMode mode;                // NOTEPAD_VIEW o NOTEPAD_EDIT
    bool modified;                   // Indicador de cambios sin guardar
    bool open;                       // Hay un documento abierto
};
```

## Uso en el Código
Para integrar nuevos tipos de archivos o modificar el comportamiento:

1. **Agregar extensiones**: Modificar `IsTextFile()` en `source/shell/notepad/notepad.cpp`
2. **Cambiar límites**: Ajustar constantes como `NOTEPAD_MAX_LINES` en `notepad.h`
3. **Agregar funciones**: Extender `Control()` para nuevos atajos
4. **Modificar UI**: Cambiar `DisplayTop()` y `DisplayBottom()` para nueva apariencia

## Consideraciones de Rendimiento
- **Memoria limitada**: Optimizado para archivos pequeños/medianos
- **Renderizado eficiente**: Solo líneas visibles se dibujan
- **Entrada controlada**: Teclado virtual evita sobrecarga

## Pendiente de revisión
- **Conflicto con el Task Manager**: SELECT es un atajo global (en `source/ui/screen_manager.cpp`) que abre el Task Manager. Por eso la acción de SELECT en modo edición (editar la línea con el teclado del sistema) no llega al editor. Hay que revisarlo.
- **Etiqueta heredada**: La pantalla inferior en modo vista todavía dice "Text Reader".

## Futuras Mejoras
- Soporte para más codificaciones
- Funciones de búsqueda y reemplazo
- Deshacer/rehacer básico
- Resaltado de sintaxis para código
- Soporte para archivos más grandes