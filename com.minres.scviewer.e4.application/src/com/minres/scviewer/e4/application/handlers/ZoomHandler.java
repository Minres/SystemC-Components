 
package com.minres.scviewer.e4.application.handlers;

import javax.inject.Named;

import org.eclipse.e4.core.di.annotations.CanExecute;
import org.eclipse.e4.core.di.annotations.Execute;
import org.eclipse.e4.ui.model.application.ui.basic.MPart;
import org.eclipse.e4.ui.workbench.modeling.EPartService;

import com.minres.scviewer.e4.application.parts.WaveformViewerPart;

public class ZoomHandler {

	final static String PARAMTER_ID="com.minres.scviewer.e4.application.command.zoomcommand.parameter.level";

	@CanExecute
	public boolean canExecute(EPartService partService) {
		return true;
	}
		
	@Execute
	public void execute(@Named(PARAMTER_ID) String level, EPartService partService) {
		MPart part = partService.getActivePart();
		Object obj = part.getObject();
		if(obj instanceof WaveformViewerPart){
			WaveformViewerPart waveformViewerPart = (WaveformViewerPart) obj;
			int zoomLevel = waveformViewerPart.getZoomLevel();
			if("in".equalsIgnoreCase(level))
				waveformViewerPart.setZoomLevel(zoomLevel-1);
			else if("out".equalsIgnoreCase(level))
				waveformViewerPart.setZoomLevel(zoomLevel+1);
			else if("fit".equalsIgnoreCase(level))
				waveformViewerPart.setZoomFit();
		}

	}
	
	
}