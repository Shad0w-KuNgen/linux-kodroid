package online.knight.mobile;

import org.libsdl.app.SDLActivity;

/**
 * Knight Online Mobile ana etkinliği. SDL'in Android köprüsü libKnightOnLine.so içindeki
 * SDL_main'i (mobile/platform/main_sdl.cpp) çağırır. İstemci verileri
 * /sdcard/Android/data/online.knight.mobile/files/ altına kopyalanmalıdır.
 */
public class KnightOnlineActivity extends SDLActivity {
    @Override
    protected String[] getLibraries() {
        return new String[] { "c++_shared", "SDL2", "KnightOnLine" };
    }
}
