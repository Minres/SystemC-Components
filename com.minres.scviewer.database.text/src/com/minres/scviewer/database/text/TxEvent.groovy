package com.minres.scviewer.database.text;

import com.minres.scviewer.database.ITx
import com.minres.scviewer.database.ITxEvent
import com.minres.scviewer.database.IWaveformEvent

class TxEvent implements ITxEvent {

	final ITxEvent.Type type;
	
	final Tx transaction;
	
	final Long time
	
	TxEvent(ITxEvent.Type type, ITx transaction) {
		super();
		this.type = type;
		this.transaction = transaction;
		this.time = type==ITxEvent.Type.BEGIN?transaction.beginTime:transaction.endTime
	}

	@Override
	IWaveformEvent duplicate() throws CloneNotSupportedException {
		new TxEvent(type, transaction, time)
	}

	@Override
	int compareTo(IWaveformEvent o) {
		time.compareTo(o.time)
	}

	@Override
	String toString() {
		type.toString()+"@"+time+" of tx #"+transaction.id;
	}
}
