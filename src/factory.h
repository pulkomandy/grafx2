/* vim:expandtab:ts=2 sw=2:
*/
void Button_Brush_Factory(void);
void Repeat_script(void);

/// Lua scripts bound to shortcut keys.
extern char * Bound_script[10];

///
/// Run a lua script linked to a shortcut, 0-9.
/// Before: Cursor hidden
/// After: Cursor shown
void Run_numbered_script(byte index);
