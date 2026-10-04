package online.knight.mobile;

import android.content.Intent;
import android.os.Bundle;

import org.libsdl.app.SDLActivity;

import java.io.File;

/**
 * Knight Online Mobile oyun etkinliği. SDL'in Android köprüsü libKnightOnLine.so içindeki
 * SDL_main'i (mobile/platform/main_sdl.cpp) çağırır. Oyun verisi yoksa önce SetupActivity
 * (zip içe aktarma) açılır. Veri dizini "--client-dir" argümanıyla yerel koda iletilir.
 */
public class KnightOnlineActivity extends SDLActivity {
    private File dataDir;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        dataDir = GameData.dataDir(this);
        if (!GameData.hasGameData(dataDir)) {
            Intent i = new Intent(this, SetupActivity.class);
            i.putExtra("setup", true);
            startActivity(i);
            finish();
            // SDLActivity.onCreate yine de çağrılmalı (super sözleşmesi); kısa ömürlü olur.
        }
        super.onCreate(savedInstanceState);
    }

    @Override
    protected String[] getLibraries() {
        return new String[] { "c++_shared", "SDL2", "KnightOnLine" };
    }

    @Override
    protected String[] getArguments() {
        if (dataDir == null)
            dataDir = GameData.dataDir(this);
        return new String[] { "--client-dir", dataDir.getAbsolutePath() };
    }
}
