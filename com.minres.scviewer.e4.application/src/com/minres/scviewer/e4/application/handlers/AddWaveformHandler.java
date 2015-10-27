
package com.minres.scviewer.e4.application.handlers;

import java.util.List;

import javax.inject.Named;

import org.eclipse.e4.core.di.annotations.CanExecute;
import org.eclipse.e4.core.di.annotations.Execute;
import org.eclipse.e4.core.di.annotations.Optional;
import org.eclipse.e4.ui.model.application.ui.basic.MPart;
import org.eclipse.e4.ui.services.IServiceConstants;
import org.eclipse.e4.ui.workbench.modeling.EPartService;
import org.eclipse.jface.viewers.IStructuredSelection;

import com.minres.scviewer.database.IWaveform;
import com.minres.scviewer.e4.application.parts.WaveformListPart;

public class AddWaveformHandler {

	public final static String PARAM_WHERE_ID="com.minres.scviewer.e4.application.command.addwaveform.where";
	public final static String PARAM_ALL_ID="com.minres.scviewer.e4.application.command.addwaveform.all";

	@CanExecute
	public boolean canExecute(@Named(PARAM_WHERE_ID) String where, @Named(PARAM_ALL_ID) String all,
			EPartService partService,
			@Named(IServiceConstants.ACTIVE_SELECTION) @Optional IStructuredSelection selection) {
		WaveformListPart listPart = getListPart( partService);
		if(listPart==null || listPart.getActiveWaveformViewerPart()==null) return false;
		Boolean before = "before".equalsIgnoreCase(where);
		if("true".equalsIgnoreCase(all)) 
			return listPart.getFilteredChildren().length>0 && 
					(!before || ((IStructuredSelection)listPart.getActiveWaveformViewerPart().getSelection()).size()>0);
		else
			return selection.size()>0 && 
					(!before || ((IStructuredSelection)listPart.getActiveWaveformViewerPart().getSelection()).size()>0);
	}

	@Execute
	public void execute(@Named(PARAM_WHERE_ID) String where, @Named(PARAM_ALL_ID) String all, 
			EPartService partService,
			@Named(IServiceConstants.ACTIVE_SELECTION) @Optional IStructuredSelection selection) {
		WaveformListPart listPart = getListPart( partService);
		if(listPart!=null && selection.size()>0){
			List<?> sel=selection.toList();
			listPart.getActiveWaveformViewerPart().addStreamsToList(sel.toArray(new IWaveform<?>[]{}),
					"before".equalsIgnoreCase(where));
		}
	}

	protected WaveformListPart getListPart(EPartService partService){
		MPart part = partService.getActivePart();
		if(part.getObject() instanceof WaveformListPart)
			return (WaveformListPart) part.getObject();
		else
			return null;
	}	
}
