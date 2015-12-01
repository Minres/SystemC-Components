/*******************************************************************************
 * Copyright (c) 2015 MINRES Technologies GmbH and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors:
 *     MINRES Technologies GmbH - initial API and implementation
 *******************************************************************************/
package com.minres.scviewer.e4.application.internal.status;
import java.lang.reflect.Method;

import org.eclipse.core.runtime.preferences.IEclipsePreferences;
import org.eclipse.core.runtime.preferences.IEclipsePreferences.IPreferenceChangeListener;
import org.eclipse.core.runtime.preferences.IEclipsePreferences.PreferenceChangeEvent;
import org.eclipse.jface.action.Action;
import org.eclipse.jface.action.IAction;
import org.eclipse.jface.action.IMenuListener;
import org.eclipse.jface.action.IMenuManager;
import org.eclipse.jface.action.MenuManager;
import org.eclipse.jface.resource.ImageDescriptor;
import org.eclipse.swt.SWT;
import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.graphics.GC;
import org.eclipse.swt.graphics.Image;
import org.eclipse.swt.graphics.Point;
import org.eclipse.swt.graphics.Rectangle;
import org.eclipse.swt.widgets.Canvas;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Event;
import org.eclipse.swt.widgets.Listener;
import org.eclipse.swt.widgets.Menu;
import org.eclipse.wb.swt.ResourceManager;
import org.eclipse.wb.swt.SWTResourceManager;
import org.osgi.service.prefs.Preferences;

/**
 * The Heap Status control, which shows the heap usage statistics in the window trim.
 * Part of the code is taken from the eclipse internal implementation
 */
public class HeapStatus extends Composite {

	/** The armed. */
	private boolean armed;
	
	/** The gc image. */
	private Image gcImage;
	
	/** The disabled gc image. */
	private Image disabledGcImage;
	
	/** The arm col. */
	private Color bgCol, usedMemCol, lowMemCol, freeMemCol, topLeftCol, bottomRightCol, sepCol, textCol, markCol, armCol;
    
    /** The button. */
    private Canvas button;
	
	/** The preferences. */
	private Preferences preferences;
	
	/** The update interval. */
	private int updateInterval;
	
	/** The show max. */
	private boolean showMax;
    
    /** The total mem. */
    private long totalMem;
    
    /** The prev total mem. */
    private long prevTotalMem = -1L;
    
    /** The prev used mem. */
    private long prevUsedMem = -1L;
    
    /** The has changed. */
    private boolean hasChanged;
    
    /** The used mem. */
    private long usedMem;
    
    /** The mark. */
    private long mark = -1;
    
    /** The img bounds. */
    // start with 12x12
	private Rectangle imgBounds = new Rectangle(0,0,12,12);
	
	/** The max mem. */
	private long maxMem = Long.MAX_VALUE;
	
	/** The max mem known. */
	private boolean maxMemKnown;
	
	/** The low mem threshold. */
	private float lowMemThreshold = 0.05f;
	
	/** The show low mem threshold. */
	private boolean showLowMemThreshold = true;
	
	/** The update tooltip. */
	private boolean updateTooltip = false;

	/** The is in gc. */
	protected volatile boolean isInGC = false;

    /** The timer. */
    private final Runnable timer = new Runnable() {
        @Override
		public void run() {
            if (!isDisposed()) {
                updateStats();
                if (hasChanged) {
                	if (updateTooltip) {
                		updateToolTip();
                	}
                    redraw();
                    hasChanged = false;
                }
                getDisplay().timerExec(updateInterval, this);
            }
        }
    };

    /** The pref listener. */
    private final IPreferenceChangeListener prefListener = new IPreferenceChangeListener() {
		@Override
		public void preferenceChange(PreferenceChangeEvent event) {
			if (IHeapStatusConstants.PREF_UPDATE_INTERVAL.equals(event.getKey())) {
				setUpdateIntervalInMS(preferences.getInt(IHeapStatusConstants.PREF_UPDATE_INTERVAL, 100));
			}
			else if (IHeapStatusConstants.PREF_SHOW_MAX.equals(event.getKey())) {
				showMax = preferences.getBoolean(IHeapStatusConstants.PREF_SHOW_MAX, true);
			}
			
		}
	};

    /**
     * Creates a new heap status control with the given parent, and using
     * the given preference store to obtain settings such as the refresh
     * interval.
     *
     * @param parent the parent composite
     * @param preferences the preference store
     */
	public HeapStatus(Composite parent, Preferences preferences) {
		super(parent, SWT.NONE);

		maxMem = getMaxMem();
		maxMemKnown = maxMem != Long.MAX_VALUE;

        this.preferences = preferences;
        if(this.preferences instanceof IEclipsePreferences)
        	((IEclipsePreferences)this.preferences).addPreferenceChangeListener(prefListener);

        setUpdateIntervalInMS(preferences.getInt(IHeapStatusConstants.PREF_UPDATE_INTERVAL, 100));
        showMax = preferences.getBoolean(IHeapStatusConstants.PREF_SHOW_MAX, true);

        button = new Canvas(this, SWT.NONE);
        button.setToolTipText("Run Garbage Collection");

		ImageDescriptor imageDesc = ResourceManager.getPluginImageDescriptor("com.minres.scviewer.e4.application", "icons/trash.png"); //$NON-NLS-1$
		Display display = getDisplay();
		gcImage = imageDesc.createImage();
		if (gcImage != null) {
			imgBounds = gcImage.getBounds();
			disabledGcImage = new Image(display, gcImage, SWT.IMAGE_DISABLE);
		}
		usedMemCol = display.getSystemColor(SWT.COLOR_INFO_BACKGROUND);
		lowMemCol = SWTResourceManager.getColor(255, 70, 70);  // medium red
		freeMemCol = SWTResourceManager.getColor(255, 190, 125);  // light orange
		bgCol = SWTResourceManager.getColor(SWT.COLOR_WIDGET_BACKGROUND);
		sepCol = topLeftCol = armCol = SWTResourceManager.getColor(SWT.COLOR_WIDGET_NORMAL_SHADOW);
		bottomRightCol = SWTResourceManager.getColor(SWT.COLOR_WIDGET_HIGHLIGHT_SHADOW);
		markCol = textCol = SWTResourceManager.getColor(SWT.COLOR_INFO_FOREGROUND);

		createContextMenu();

        Listener listener = new Listener() {

            @Override
			public void handleEvent(Event event) {
                switch (event.type) {
                case SWT.Dispose:
                	doDispose();
                    break;
                case SWT.Resize:
                    Rectangle rect = getClientArea();
                    button.setBounds(rect.width - imgBounds.width - 1, 1, imgBounds.width, rect.height - 2);
                    break;
                case SWT.Paint:
                    if (event.widget == HeapStatus.this) {
                    	paintComposite(event.gc);
                    }
                    else if (event.widget == button) {
                        paintButton(event.gc);
                    }
                    break;
                case SWT.MouseUp:
                    if (event.button == 1) {
						if (!isInGC) {
							arm(false);
							gc();
						}
                    }
                    break;
                case SWT.MouseDown:
                    if (event.button == 1) {
	                    if (event.widget == HeapStatus.this) {
							setMark();
						} else if (event.widget == button) {
							if (!isInGC)
								arm(true);
						}
                    }
                    break;
                case SWT.MouseEnter:
                	HeapStatus.this.updateTooltip = true;
                	updateToolTip();
                	break;
                case SWT.MouseExit:
                    if (event.widget == HeapStatus.this) {
                    	HeapStatus.this.updateTooltip = false;
					} else if (event.widget == button) {
						arm(false);
					}
                    break;
                }
            }

        };
        addListener(SWT.Dispose, listener);
        addListener(SWT.MouseDown, listener);
        addListener(SWT.Paint, listener);
        addListener(SWT.Resize, listener);
        addListener(SWT.MouseEnter, listener);
        addListener(SWT.MouseExit, listener);
        button.addListener(SWT.MouseDown, listener);
        button.addListener(SWT.MouseExit, listener);
        button.addListener(SWT.MouseUp, listener);
        button.addListener(SWT.Paint, listener);

		// make sure stats are updated before first paint
		updateStats();

        getDisplay().asyncExec(new Runnable() {
			@Override
			public void run() {
				if (!isDisposed()) {
					getDisplay().timerExec(updateInterval, timer);
				}
			}
		});
   	}

	/* (non-Javadoc)
	 * @see org.eclipse.swt.widgets.Control#setBackground(org.eclipse.swt.graphics.Color)
	 */
	@Override
	public void setBackground(Color color) {
		bgCol = color;
		button.redraw();
		button.update();
	}

	/* (non-Javadoc)
	 * @see org.eclipse.swt.widgets.Control#setForeground(org.eclipse.swt.graphics.Color)
	 */
	@Override
	public void setForeground(Color color) {
		if (color == null) {
			usedMemCol = getDisplay().getSystemColor(SWT.COLOR_INFO_BACKGROUND);
		} else {
			usedMemCol = color;
		}

		button.redraw();
		button.update();
	}

	/* (non-Javadoc)
	 * @see org.eclipse.swt.widgets.Control#getForeground()
	 */
	@Override
	public Color getForeground() {
		if (usedMemCol != null) {
			return usedMemCol;
		}
		return getDisplay().getSystemColor(SWT.COLOR_INFO_BACKGROUND);
	}

	/**
	 * Returns the maximum memory limit, or Long.MAX_VALUE if the max is not known.
	 *
	 * @return the max mem
	 */
	private long getMaxMem() {
		long max = Long.MAX_VALUE;
		try {
			// Must use reflect to allow compilation against JCL/Foundation
			Method maxMemMethod = Runtime.class.getMethod("maxMemory", new Class[0]); //$NON-NLS-1$
			Object o = maxMemMethod.invoke(Runtime.getRuntime(), new Object[0]);
			if (o instanceof Long) {
				max = ((Long) o).longValue();
			}
		}
		catch (Exception e) {
			// ignore if method missing or if there are other failures trying to determine the max
		}
		return max;
	}

	/**
	 * Sets the update interval in ms.
	 *
	 * @param interval the new update interval in ms
	 */
	private void setUpdateIntervalInMS(int interval) {
		updateInterval = Math.max(100, interval);
	}

	/**
	 * Do dispose.
	 */
	private void doDispose() {
		if(preferences instanceof IEclipsePreferences)
        	((IEclipsePreferences)preferences).removePreferenceChangeListener(prefListener);
    	if (gcImage != null) {
			gcImage.dispose();
		}
		if (disabledGcImage != null) {
			disabledGcImage.dispose();
		}
	}

	/* (non-Javadoc)
	 * @see org.eclipse.swt.widgets.Composite#computeSize(int, int, boolean)
	 */
	@Override
	public Point computeSize(int wHint, int hHint, boolean changed) {
        GC gc = new GC(this);
        Point p = gc.textExtent("MMMMMMMMMMMM");
        int height = imgBounds.height;
        // choose the largest of
        // 	- Text height + margins
        //	- Image height + margins
        //	- Default Trim heightin
        height = Math.max(height, p.y) + 4;
        height = Math.max(TrimUtil.TRIM_DEFAULT_HEIGHT, height);
        gc.dispose();
		return new Point(p.x + 15, height);
	}

    /**
     * Arm.
     *
     * @param armed the armed
     */
    private void arm(boolean armed) {
        if (this.armed == armed) {
			return;
		}
        this.armed = armed;
        button.redraw();
        button.update();
    }

	/**
	 * Gc running.
	 *
	 * @param isInGC the is in gc
	 */
	private void gcRunning(boolean isInGC) {
		if (this.isInGC == isInGC) {
			return;
		}
		this.isInGC = isInGC;
		 button.redraw();
		 button.update();
	}

    /**
     * Creates the context menu.
     */
    private void createContextMenu() {
        MenuManager menuMgr = new MenuManager();
        menuMgr.setRemoveAllWhenShown(true);
        menuMgr.addMenuListener(new IMenuListener() {
			@Override
			public void menuAboutToShow(IMenuManager menuMgr) {
				fillMenu(menuMgr);
			}
		});
        Menu menu = menuMgr.createContextMenu(this);
        setMenu(menu);
    }

    /**
     * Fill menu.
     *
     * @param menuMgr the menu mgr
     */
    private void fillMenu(IMenuManager menuMgr) {
        menuMgr.add(new SetMarkAction());
        menuMgr.add(new ClearMarkAction());
        menuMgr.add(new ShowMaxAction());
        menuMgr.add(new CloseHeapStatusAction());
//        if (isKyrsoftViewAvailable()) {
//        	menuMgr.add(new ShowKyrsoftViewAction());
//        }
    }

    /**
     * Sets the mark to the current usedMem level.
     */
    private void setMark() {
    	updateStats();  // get up-to-date stats before taking the mark
        mark = usedMem;
        hasChanged = true;
        redraw();
    }

    /**
     * Clears the mark.
     */
    private void clearMark() {
        mark = -1;
        hasChanged = true;
        redraw();
    }

    /**
     * Gc.
     */
    private void gc() {
		gcRunning(true);
		Thread t = new Thread() {
			@Override
			public void run() {
				busyGC();
				getDisplay().asyncExec(new Runnable() {
					@Override
					public void run() {
						if (!isDisposed()) {
							gcRunning(false);
						}
					}
				});
			}
		};
		t.start();
    }

    /**
     * Busy gc.
     */
    private void busyGC() {
        for (int i = 0; i < 2; ++i) {
	        System.gc();
	        System.runFinalization();
        }
    }

    /**
     * Paint button.
     *
     * @param gc the gc
     */
    private void paintButton(GC gc) {
        Rectangle rect = button.getClientArea();
		if (isInGC) {
			if (disabledGcImage != null) {
				int buttonY = (rect.height - imgBounds.height) / 2 + rect.y;
				gc.drawImage(disabledGcImage, rect.x, buttonY);
			}
			return;
		}
        if (armed) {
            gc.setBackground(armCol);
            gc.fillRectangle(rect.x, rect.y, rect.width, rect.height);
        }
        if (gcImage != null) {
			int by = (rect.height - imgBounds.height) / 2 + rect.y; // button y
			gc.drawImage(gcImage, rect.x, by);
        }
    }

    /**
     * Paint composite.
     *
     * @param gc the gc
     */
    private void paintComposite(GC gc) {
		if (showMax && maxMemKnown) {
			paintCompositeMaxKnown(gc);
		} else {
			paintCompositeMaxUnknown(gc);
		}
    }

    /**
     * Paint composite max unknown.
     *
     * @param gc the gc
     */
    private void paintCompositeMaxUnknown(GC gc) {
        Rectangle rect = getClientArea();
        int x = rect.x;
        int y = rect.y;
        int w = rect.width;
        int h = rect.height;
        int bw = imgBounds.width; // button width
        int dx = x + w - bw - 2; // divider x
        int sw = w - bw - 3; // status width
        int uw = (int) (sw * usedMem / totalMem); // used mem width
        int ux = x + 1 + uw; // used mem right edge
        if (bgCol != null) {
			gc.setBackground(bgCol);
		}
        gc.fillRectangle(rect);
        gc.setForeground(sepCol);
		gc.drawLine(dx, y, dx, y + h);
		gc.drawLine(ux, y, ux, y + h);
        gc.setForeground(topLeftCol);
        gc.drawLine(x, y, x+w, y);
		gc.drawLine(x, y, x, y+h);
		gc.setForeground(bottomRightCol);
        gc.drawLine(x+w-1, y, x+w-1, y+h);
		gc.drawLine(x, y+h-1, x+w, y+h-1);

		gc.setBackground(usedMemCol);
        gc.fillRectangle(x + 1, y + 1, uw, h - 2);

        String s =  convertToMegString(usedMem)+" of "+ convertToMegString(totalMem);
        Point p = gc.textExtent(s);
        int sx = (rect.width - 15 - p.x) / 2 + rect.x + 1;
        int sy = (rect.height - 2 - p.y) / 2 + rect.y + 1;
        gc.setForeground(textCol);
        gc.drawString(s, sx, sy, true);

        // draw an I-shaped bar in the foreground colour for the mark (if present)
        if (mark != -1) {
            int ssx = (int) (sw * mark / totalMem) + x + 1;
            paintMark(gc, ssx, y, h);
        }
    }

    /**
     * Paint composite max known.
     *
     * @param gc the gc
     */
    private void paintCompositeMaxKnown(GC gc) {
        Rectangle rect = getClientArea();
        int x = rect.x;
        int y = rect.y;
        int w = rect.width;
        int h = rect.height;
        int bw = imgBounds.width; // button width
        int dx = x + w - bw - 2; // divider x
        int sw = w - bw - 3; // status width
        int uw = (int) (sw * usedMem / maxMem); // used mem width
        int ux = x + 1 + uw; // used mem right edge
        int tw = (int) (sw * totalMem / maxMem); // current total mem width
        int tx = x + 1 + tw; // current total mem right edge

        if (bgCol != null) {
			gc.setBackground(bgCol);
		}
        gc.fillRectangle(rect);
        gc.setForeground(sepCol);
		gc.drawLine(dx, y, dx, y + h);
		gc.drawLine(ux, y, ux, y + h);
		gc.drawLine(tx, y, tx, y + h);
        gc.setForeground(topLeftCol);
        gc.drawLine(x, y, x+w, y);
		gc.drawLine(x, y, x, y+h);
		gc.setForeground(bottomRightCol);
        gc.drawLine(x+w-1, y, x+w-1, y+h);
		gc.drawLine(x, y+h-1, x+w, y+h-1);

        if (lowMemThreshold != 0 && ((double)(maxMem - usedMem) / (double)maxMem < lowMemThreshold)) {
            gc.setBackground(lowMemCol);
        } else {
            gc.setBackground(usedMemCol);
        }
        gc.fillRectangle(x + 1, y + 1, uw, h - 2);

        gc.setBackground(freeMemCol);
        gc.fillRectangle(ux + 1, y + 1, tx - (ux + 1), h - 2);

        // paint line for low memory threshold
        if (showLowMemThreshold && lowMemThreshold != 0) {
            gc.setForeground(lowMemCol);
            int thresholdX = x + 1 + (int) (sw * (1.0 - lowMemThreshold));
            gc.drawLine(thresholdX, y + 1, thresholdX, y + h - 2);
        }

        String s = convertToMegString(usedMem)+" of "+convertToMegString(totalMem);
        Point p = gc.textExtent(s);
        int sx = (rect.width - 15 - p.x) / 2 + rect.x + 1;
        int sy = (rect.height - 2 - p.y) / 2 + rect.y + 1;
        gc.setForeground(textCol);
        gc.drawString(s, sx, sy, true);

        // draw an I-shaped bar in the foreground colour for the mark (if present)
        if (mark != -1) {
            int ssx = (int) (sw * mark / maxMem) + x + 1;
            paintMark(gc, ssx, y, h);
        }
    }

	/**
	 * Paint mark.
	 *
	 * @param gc the gc
	 * @param x the x
	 * @param y the y
	 * @param h the h
	 */
	private void paintMark(GC gc, int x, int y, int h) {
        gc.setForeground(markCol);
		gc.drawLine(x, y+1, x, y+h-2);
		gc.drawLine(x-1, y+1, x+1, y+1);
		gc.drawLine(x-1, y+h-2, x+1, y+h-2);
	}

    /**
     * Update stats.
     */
    private void updateStats() {
        Runtime runtime = Runtime.getRuntime();
        totalMem = runtime.totalMemory();
        long freeMem = runtime.freeMemory();
        usedMem = totalMem - freeMem;

        if (convertToMeg(prevUsedMem) != convertToMeg(usedMem)) {
            prevUsedMem = usedMem;
            this.hasChanged = true;
        }

        if (prevTotalMem != totalMem) {
            prevTotalMem = totalMem;
            this.hasChanged = true;
        }
    }

    /**
     * Update tool tip.
     */
    private void updateToolTip() {
    	String usedStr = convertToMegString(usedMem);
    	String totalStr = convertToMegString(totalMem);
    	String maxStr = maxMemKnown ? convertToMegString(maxMem) : "<unknown>";
    	String markStr = mark == -1 ? "<none>" : convertToMegString(mark);
        String toolTip = "Heap size: "+usedStr+" of total: "+totalStr+" max: "+maxStr+" mark: "+markStr;
        if (!toolTip.equals(getToolTipText())) {
            setToolTipText(toolTip);
        }
    }

    /**
     * Converts the given number of bytes to a printable number of megabytes (rounded up).
     *
     * @param numBytes the num bytes
     * @return the string
     */
    private String convertToMegString(long numBytes) {
        return new Long(convertToMeg(numBytes)).toString()+"M";
    }

    /**
     * Converts the given number of bytes to the corresponding number of megabytes (rounded up).
     *
     * @param numBytes the num bytes
     * @return the long
     */
	private long convertToMeg(long numBytes) {
		return (numBytes + (512 * 1024)) / (1024 * 1024);
	}


    /**
     * The Class SetMarkAction.
     */
    class SetMarkAction extends Action {
        
        /**
         * Instantiates a new sets the mark action.
         */
        SetMarkAction() {
            super("&Set Mark");
        }

        /* (non-Javadoc)
         * @see org.eclipse.jface.action.Action#run()
         */
        @Override
		public void run() {
            setMark();
        }
    }

    /**
     * The Class ClearMarkAction.
     */
    class ClearMarkAction extends Action {
        
        /**
         * Instantiates a new clear mark action.
         */
        ClearMarkAction() {
            super("&Clear Mark");
        }

        /* (non-Javadoc)
         * @see org.eclipse.jface.action.Action#run()
         */
        @Override
		public void run() {
            clearMark();
        }
    }

    /**
     * The Class ShowMaxAction.
     */
    class ShowMaxAction extends Action {
    	
	    /**
	     * Instantiates a new show max action.
	     */
	    ShowMaxAction() {
            super("Show &Max Heap", IAction.AS_CHECK_BOX);
            setEnabled(maxMemKnown);
            setChecked(showMax);
        }

        /* (non-Javadoc)
         * @see org.eclipse.jface.action.Action#run()
         */
        @Override
		public void run() {
            preferences.putBoolean(IHeapStatusConstants.PREF_SHOW_MAX, isChecked());
            redraw();
        }
    }

    /**
     * The Class CloseHeapStatusAction.
     */
    class CloseHeapStatusAction extends Action{

    	/**
	     * Instantiates a new close heap status action.
	     */
	    CloseHeapStatusAction(){
    		super("&Close");
    	}

    	/* (non-Javadoc)
	     * @see org.eclipse.jface.action.Action#run()
	     */
	    @Override
		public void run(){
//			WorkbenchWindow wbw = (WorkbenchWindow) PlatformUI.getWorkbench()
//					.getActiveWorkbenchWindow();
//			if (wbw != null) {
//				wbw.showHeapStatus(false);
//			}
    		System.out.println("NYI");
    	}
    }

}

