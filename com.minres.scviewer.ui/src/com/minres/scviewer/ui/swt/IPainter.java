package com.minres.scviewer.ui.swt;

import org.eclipse.swt.graphics.GC;
import org.eclipse.swt.graphics.Image;
import org.eclipse.swt.graphics.Rectangle;

public interface IPainter {

	void paintArea(GC gc,Rectangle area);
	
}