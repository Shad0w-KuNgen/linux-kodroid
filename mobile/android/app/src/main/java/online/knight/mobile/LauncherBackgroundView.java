package online.knight.mobile;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.LinearGradient;
import android.graphics.Paint;
import android.graphics.RadialGradient;
import android.graphics.Rect;
import android.graphics.RectF;
import android.graphics.Shader;
import android.view.View;

import java.io.File;

/**
 * Launcher arka planı. Veri dizininde launcher_bg.png/.jpg (ya da Launcher/background.*) varsa
 * onu ekranı dolduracak şekilde çizer (sunucu işletmecisi kendi KO görselini koyabilir); yoksa
 * koyu, altın parıltılı bir kale/gece atmosferi çizilir. Üstüne okunabilirlik için karartma.
 */
public class LauncherBackgroundView extends View {
    private Bitmap bitmap;
    private final Paint paint = new Paint(Paint.ANTI_ALIAS_FLAG | Paint.FILTER_BITMAP_FLAG);

    public LauncherBackgroundView(Context ctx, File dataDir) {
        super(ctx);
        String[] candidates = { "launcher_bg.png", "launcher_bg.jpg", "Launcher/background.png",
                "Launcher/background.jpg", "launcher/background.png", "launcher/background.jpg" };
        for (String c : candidates) {
            File f = new File(dataDir, c);
            if (f.isFile()) {
                try {
                    BitmapFactory.Options o = new BitmapFactory.Options();
                    o.inPreferredConfig = Bitmap.Config.RGB_565;
                    bitmap = BitmapFactory.decodeFile(f.getPath(), o);
                } catch (Throwable ignored) {
                }
                if (bitmap != null)
                    break;
            }
        }
    }

    @Override
    protected void onDraw(Canvas c) {
        int w = getWidth(), h = getHeight();
        if (w <= 0 || h <= 0)
            return;
        if (bitmap != null) {
            // center-crop
            float scale = Math.max((float) w / bitmap.getWidth(), (float) h / bitmap.getHeight());
            int dw = Math.round(bitmap.getWidth() * scale), dh = Math.round(bitmap.getHeight() * scale);
            int dx = (w - dw) / 2, dy = (h - dh) / 2;
            c.drawBitmap(bitmap, new Rect(0, 0, bitmap.getWidth(), bitmap.getHeight()), new Rect(dx, dy, dx + dw, dy + dh), paint);
            paint.setShader(null);
            paint.setColor(0x88000000);
            c.drawRect(0, 0, w, h, paint);
            return;
        }
        // Gece gökyüzü → toprak: dikey geçiş
        paint.setShader(new LinearGradient(0, 0, 0, h,
                new int[] { 0xFF1B1A2E, 0xFF2A1F1A, 0xFF120E0A, 0xFF050403 },
                new float[] { 0f, 0.45f, 0.8f, 1f }, Shader.TileMode.CLAMP));
        c.drawRect(0, 0, w, h, paint);
        // Ufukta altın parıltı (sağ üst)
        paint.setShader(new RadialGradient(w * 0.78f, h * 0.18f, w * 0.55f,
                new int[] { 0x66E0B35A, 0x22A66A1E, 0x00000000 }, new float[] { 0f, 0.4f, 1f },
                Shader.TileMode.CLAMP));
        c.drawRect(0, 0, w, h, paint);
        // Uzak kale silueti: basit kuleler
        paint.setShader(null);
        paint.setColor(0xFF0B0908);
        float base = h * 0.82f;
        float[][] towers = { { 0.05f, 0.62f, 0.07f }, { 0.14f, 0.55f, 0.05f }, { 0.22f, 0.66f, 0.09f },
                { 0.60f, 0.70f, 0.06f }, { 0.70f, 0.58f, 0.08f }, { 0.82f, 0.64f, 0.05f }, { 0.92f, 0.72f, 0.07f } };
        for (float[] t : towers) {
            float x = w * t[0], top = h * t[1], tw = w * t[2];
            c.drawRect(x, top, x + tw, base, paint);
            // mazgallar
            float m = tw / 5f;
            for (int i = 0; i < 5; i += 2)
                c.drawRect(x + i * m, top - m, x + (i + 1) * m, top, paint);
        }
        c.drawRect(0, base - h * 0.04f, w, h, paint);
        // Zemin sisi
        paint.setShader(new LinearGradient(0, base - h * 0.15f, 0, h,
                new int[] { 0x00000000, 0x55100C08, 0xCC000000 }, null, Shader.TileMode.CLAMP));
        c.drawRect(0, base - h * 0.15f, w, h, paint);
        // Vinyet
        paint.setShader(new RadialGradient(w / 2f, h / 2f, Math.max(w, h) * 0.7f,
                new int[] { 0x00000000, 0x00000000, 0x99000000 }, new float[] { 0f, 0.55f, 1f },
                Shader.TileMode.CLAMP));
        c.drawRect(0, 0, w, h, paint);
        paint.setShader(null);
        // Hafif kenar çerçevesi
        paint.setStyle(Paint.Style.STROKE);
        paint.setStrokeWidth(2f);
        paint.setColor(0x33C9A24C);
        c.drawRect(new RectF(6, 6, w - 6, h - 6), paint);
        paint.setStyle(Paint.Style.FILL);
        paint.setColor(Color.WHITE);
    }
}
