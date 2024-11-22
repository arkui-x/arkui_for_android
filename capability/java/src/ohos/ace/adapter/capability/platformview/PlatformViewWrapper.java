/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

 package ohos.ace.adapter.capability.platformview;

 import android.app.Activity;
 import android.content.Context;
 import android.graphics.Canvas;
 import android.graphics.Color;
 import android.graphics.Matrix;
 import android.graphics.PorterDuff;
 import android.graphics.Rect;
 
 import android.view.MotionEvent;
 import android.view.View;
 import android.view.ViewParent;
 import android.view.Surface;
 import android.widget.FrameLayout;
 
 import java.util.HashMap;
 import java.util.Map;
 import java.util.concurrent.atomic.AtomicLong;
 
 import ohos.ace.adapter.AceResourcePlugin;
 import ohos.ace.adapter.capability.texture.AceTexture;
 import ohos.ace.adapter.ALog;
 
 /**
  * The class for creating PlatformViewWrapper on Android platform.
  *
  * @since 1
  */
 public class PlatformViewWrapper extends FrameLayout {
     private static final String LOG_TAG = "PlatformViewWrapper";
 
     private int left;
     private int top;
 
     private int bufferWidth = 0;
     private int bufferHeight = 0;
 
     private Surface surface;
     private Context context;
 
     private FrameLayout.LayoutParams buildLayoutParams(int left, int top, int width, int height) {
         FrameLayout.LayoutParams params = new FrameLayout.LayoutParams(width, height);
         params.setMargins(left, top, width, height);
         return params;
     }
 
     /**
      * constructor of PlatformViewWrapper
      *
      */
     public PlatformViewWrapper(Context context) {
         super(context);
         setWillNotDraw(false);
 
         surface = null;
         this.context = context;
     }
 
     public void displayPlatformView() {
         FrameLayout.LayoutParams layoutParams = buildLayoutParams(left, top, bufferWidth, bufferHeight);
         setLayoutParams(layoutParams);
         Activity activity = (Activity) context;
         this.setZ(-1.f);
         activity.addContentView(this, layoutParams);
     }
 
     /**
      * This is set Surface.
      *
      * @return void
      */
     public void setSurface(Surface newsurface) {
         surface = newsurface;
     }
 
     /**
      * This is set layout parameters for view.
      *
      * @param params LayoutParams
      * @return void
      */
     public void setLayoutparams(FrameLayout.LayoutParams params) {
         super.setLayoutParams(params);
         
         left = params.leftMargin;
         top = params.topMargin;
     }
 
     // be Called when render should be resized.
     public void resize(int width, int height) {
         bufferWidth = width;
         bufferHeight = height;
     }
 
     /**
      * This is Intercept TouchEvent.
      * 
      * @return boolean
      */
     @Override
     public boolean onInterceptTouchEvent(MotionEvent event) {
         return true;
     }
 
     @Override
     public void onDescendantInvalidated(View child, View target) {
         super.onDescendantInvalidated(child, target);
         invalidate();
     }
 
     @Override
     public ViewParent invalidateChildInParent(int[] location, Rect dirty) {
         invalidate();
         return super.invalidateChildInParent(location, dirty);
     }
 
     /**
      * This is draw.
      *
      * @param canvas Canvas object
      * @return void
      */
     @Override
     public void draw(Canvas canvas) {
         if (surface == null) {
             super.draw(canvas);
             ALog.e(LOG_TAG, "PlatformView cannot be composed without a Render.");
             return;
         }
         final Canvas newCanvas = surface.lockHardwareCanvas();
         if (newCanvas == null) {
             invalidate();
             return;
         }
         try {
             newCanvas.drawColor(Color.TRANSPARENT, PorterDuff.Mode.CLEAR);
             super.draw(newCanvas);
         } finally {
             surface.unlockCanvasAndPost(newCanvas);
         }
     }
 }