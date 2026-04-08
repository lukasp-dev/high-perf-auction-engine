import random
import csv
import time

def generate_data(num_artworks=10000, num_bids=10000000):
    artworks = []
    artwork_ids = []
    
    print(f"Generating {num_artworks} artworks...")
    with open('artworks.csv', 'w', newline='') as f:
        writer = csv.writer(f)
        for i in range(num_artworks):
            art_id = f"ART-{i:06d}"
            artist_id = f"ARTIST-{random.randint(1, 500):03d}"
            royalty = round(random.uniform(0.01, 0.15), 2)
            base_price = random.randint(100, 10000)
            writer.writerow([art_id, artist_id, royalty, base_price])
            artwork_ids.append(art_id)

    print(f"Generating {num_bids} bids...")
    start_time = int(time.time() * 1000)
    with open('bids.csv', 'w', newline='') as f:
        writer = csv.writer(f)
        for i in range(num_bids):
            art_id = random.choice(artwork_ids)
            bidder_id = f"BIDDER-{random.randint(1, 50000):05d}"
            # Increment price slightly or stay same
            price = random.randint(1000, 20000) + random.random()
            timestamp = start_time + i # 1ms increments
            writer.writerow([art_id, bidder_id, f"{price:.2f}", timestamp])

if __name__ == "__main__":
    generate_data()
