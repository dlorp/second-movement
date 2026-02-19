// decoder.js - FESK binary decoder & data management
// GameBoy DMG palette + demoscene precision

class FESKDecoder {
    constructor() {
        this.db = null;
        this.initDB();
    }

    // IndexedDB initialization
    async initDB() {
        return new Promise((resolve, reject) => {
            const request = indexedDB.open('FESKData', 1);
            
            request.onerror = () => reject(request.error);
            request.onsuccess = () => {
                this.db = request.result;
                resolve();
            };
            
            request.onupgradeneeded = (e) => {
                const db = e.target.result;
                if (!db.objectStoreNames.contains('sleepData')) {
                    const store = db.createObjectStore('sleepData', { keyPath: 'timestamp' });
                    store.createIndex('timestamp', 'timestamp', { unique: false });
                }
            };
        });
    }

    // Hex string → byte array
    hexToBytes(hex) {
        const clean = hex.replace(/[^0-9A-Fa-f]/g, '');
        if (clean.length !== 224) {
            throw new Error(`Invalid hex length: ${clean.length} (expected 224)`);
        }
        
        const bytes = new Uint8Array(112);
        for (let i = 0; i < 112; i++) {
            bytes[i] = parseInt(clean.substr(i * 2, 2), 16);
        }
        return bytes;
    }

    // Parse 7 nights from 112 bytes (16 bytes per night)
    parseNights(bytes) {
        const nights = [];
        const view = new DataView(bytes.buffer);
        
        for (let i = 0; i < 7; i++) {
            const offset = i * 16;
            
            const night = {
                onset: view.getUint32(offset, true),           // Little-endian
                offset: view.getUint32(offset + 4, true),
                duration: view.getUint16(offset + 8, true),
                efficiency: view.getUint8(offset + 10),
                waso: view.getUint16(offset + 11, true),
                awakenings: view.getUint8(offset + 13),
                light: view.getUint8(offset + 14),
                valid: view.getUint8(offset + 15)
            };
            
            // Calculate derived metrics
            if (night.valid && night.onset > 0) {
                night.onsetTime = new Date(night.onset * 1000);
                night.offsetTime = new Date(night.offset * 1000);
                night.durationHours = (night.duration / 60).toFixed(1);
                night.quality = this.calculateQuality(night);
            }
            
            nights.push(night);
        }
        
        return nights;
    }

    // Calculate quality score (0-100) from metrics
    calculateQuality(night) {
        if (!night.valid) return 0;
        
        // Weighted formula:
        // 40% efficiency
        // 30% duration (optimal 7-9h)
        // 20% awakenings (fewer is better)
        // 10% WASO (lower is better)
        
        const effScore = night.efficiency;
        
        const hours = night.duration / 60;
        const durScore = hours >= 7 && hours <= 9 ? 100 : 
                         hours >= 6 && hours < 7 ? 80 :
                         hours > 9 && hours <= 10 ? 80 : 60;
        
        const awakeScore = Math.max(0, 100 - (night.awakenings * 15));
        const wasoScore = Math.max(0, 100 - (night.waso / 2));
        
        return Math.round(
            effScore * 0.4 +
            durScore * 0.3 +
            awakeScore * 0.2 +
            wasoScore * 0.1
        );
    }

    // Calculate Circadian Score components from 7 nights
    calculateCircadianScore(nights) {
        const valid = nights.filter(n => n.valid && n.onset > 0);
        if (valid.length < 3) {
            return { cs: 0, sri: 0, duration: 0, efficiency: 0, compliance: 0, light: 0 };
        }

        // SRI: Sleep Regularity Index (onset time consistency)
        const onsetHours = valid.map(n => {
            const d = new Date(n.onset * 1000);
            return d.getHours() + d.getMinutes() / 60;
        });
        const meanOnset = onsetHours.reduce((a, b) => a + b) / onsetHours.length;
        const variance = onsetHours.reduce((sum, h) => sum + Math.pow(h - meanOnset, 2), 0) / onsetHours.length;
        const sri = Math.max(0, Math.min(100, 100 - (variance * 10)));

        // Duration score (target 7-9h)
        const avgDuration = valid.reduce((sum, n) => sum + n.duration, 0) / valid.length / 60;
        const duration = avgDuration >= 7 && avgDuration <= 9 ? 100 :
                        avgDuration >= 6 && avgDuration < 7 ? 85 :
                        avgDuration > 9 && avgDuration <= 10 ? 85 : 60;

        // Efficiency score
        const efficiency = Math.round(valid.reduce((sum, n) => sum + n.efficiency, 0) / valid.length);

        // Compliance: valid nights / 7
        const compliance = Math.round((valid.length / 7) * 100);

        // Light exposure (normalized)
        const light = Math.round(valid.reduce((sum, n) => sum + n.light, 0) / valid.length / 2.55);

        // Overall CS: weighted average
        const cs = Math.round(
            sri * 0.25 +
            duration * 0.25 +
            efficiency * 0.25 +
            compliance * 0.15 +
            light * 0.10
        );

        return { cs, sri, duration, efficiency, compliance, light };
    }

    // Store decoded data in IndexedDB
    async storeData(nights, circadianScore) {
        if (!this.db) await this.initDB();
        
        const transaction = this.db.transaction(['sleepData'], 'readwrite');
        const store = transaction.objectStore('sleepData');
        
        const record = {
            timestamp: Date.now(),
            nights: nights,
            circadianScore: circadianScore,
            receivedAt: new Date().toISOString()
        };
        
        await store.add(record);
        return record;
    }

    // Retrieve all stored records
    async getAllData() {
        if (!this.db) await this.initDB();
        
        return new Promise((resolve, reject) => {
            const transaction = this.db.transaction(['sleepData'], 'readonly');
            const store = transaction.objectStore('sleepData');
            const request = store.getAll();
            
            request.onerror = () => reject(request.error);
            request.onsuccess = () => resolve(request.result);
        });
    }

    // Export to CSV
    exportCSV(nights, circadianScore) {
        let csv = 'Date,Onset,Offset,Duration(h),Efficiency(%),WASO(min),Awakenings,Light,Quality\n';
        
        nights.forEach((n, i) => {
            if (n.valid && n.onset > 0) {
                const date = new Date(n.onset * 1000).toISOString().split('T')[0];
                const onset = new Date(n.onset * 1000).toTimeString().substr(0, 5);
                const offset = new Date(n.offset * 1000).toTimeString().substr(0, 5);
                csv += `${date},${onset},${offset},${n.durationHours},${n.efficiency},${n.waso},${n.awakenings},${n.light},${n.quality}\n`;
            }
        });
        
        csv += `\nCircadian Score,${circadianScore.cs}\n`;
        csv += `SRI,${circadianScore.sri}\n`;
        csv += `Duration,${circadianScore.duration}\n`;
        csv += `Efficiency,${circadianScore.efficiency}\n`;
        csv += `Compliance,${circadianScore.compliance}\n`;
        csv += `Light,${circadianScore.light}\n`;
        
        return csv;
    }

    // Export to JSON
    exportJSON(nights, circadianScore) {
        return JSON.stringify({
            version: '1.0',
            exportTime: new Date().toISOString(),
            nights: nights,
            circadianScore: circadianScore
        }, null, 2);
    }

    // Download helper
    downloadFile(content, filename, mimeType) {
        const blob = new Blob([content], { type: mimeType });
        const url = URL.createObjectURL(blob);
        const a = document.createElement('a');
        a.href = url;
        a.download = filename;
        a.click();
        URL.revokeObjectURL(url);
    }
}
